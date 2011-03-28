// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QtXml>
#include <QHostInfo>

#include "rapidxml/rapidxml.hpp"

#include "Common/BoostFilesystem.hpp"

#include "Common/BasicExceptions.hpp"
#include "Common/MPI/PE.hpp"
#include "Common/Log.hpp"
#include "Common/Signal.hpp"

#include "Common/XML/Protocol.hpp"

#include "GUI/UICommon/ComponentNames.hpp"

#include "GUI/Server/RemoteClientAppender.hpp"
#include "GUI/Server/ServerNetworkComm.hpp"

#include "GUI/Server/CCore.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::GUI::UICommon;
using namespace CF::GUI::Server;

CCore::CCore()
  : Component(SERVER_CORE),
    DEFAULT_PATH("."),
    m_fileOpen(false),
    m_simRunning(false),
    m_active(false)
{


  TypeInfo::instance().regist<CCore>( type_name() );

  m_commServer = new ServerNetworkComm();

  connect(m_commServer, SIGNAL(newClientConnected(std::string)),
          this,  SLOT(newClient(std::string)));

  RemoteClientAppender * rca = new RemoteClientAppender();

  Logger::instance().getStream(Logger::WARN).addStringForwarder(rca);
  Logger::instance().getStream(Logger::ERROR).addStringForwarder(rca);
  Logger::instance().getStream(Logger::INFO).addStringForwarder(rca);

  Logger::instance().getStream(Logger::INFO).setStamp(LogStream::STRING, "%type% ");
  Logger::instance().getStream(Logger::ERROR).setStamp(LogStream::STRING, "%type% ");
  Logger::instance().getStream(Logger::WARN).setStamp(LogStream::STRING, "%type% ");

  connect(rca, SIGNAL(newData(QString)), this, SLOT(message(QString)));

  regist_signal("read_dir", "Read directory content")->
      signal->connect(boost::bind(&CCore::read_dir, this, _1));
  regist_signal("shutdown", "Shutdown the server")->
      signal->connect(boost::bind(&CCore::shutdown, this, _1));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.+++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CCore::~CCore()
{
  delete m_commServer;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool CCore::listenToPort(quint16 portNumber)
{
  return m_commServer->openPort(portNumber);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::sendSignal( const XmlDoc & signal )
{
  m_commServer->sendSignalToClient(signal);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::sendFrameRejected(const std::string & clientid,
                              const std::string & frameid,
                              const CF::Common::URI & sender,
                              const QString & reason)
{
  m_commServer->sendFrameRejectedToClient(clientid, frameid, sender, reason);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::sendException(const char * what,
                          const std::string & clientid)
{
  m_commServer->sendMessageToClient(what, LogMessage::EXCEPTION, clientid);
}

/***************************************************************************

PRIVATE METHODS

***************************************************************************/

bool CCore::getDirContent(const QString & directory,
                          const std::vector<std::string> & extensions,
                          bool includeFiles,
                          bool includeNoExtension,
                          std::vector<std::string> & dirsList,
                          std::vector<std::string> & filesList) const
{
  QStringList list;
  QDir dir(directory);
  bool dirExists = dir.exists();

  dir.setFilter(QDir::Files | QDir::Dirs | QDir::Hidden);
  dir.setSorting(QDir::DirsFirst | QDir::Name);

  if(dirExists)
  {
    QFileInfoList files = dir.entryInfoList();
    QFileInfoList::iterator it = files.begin();

    QRegExp regex("", Qt::CaseSensitive, QRegExp::RegExp);

    if(!extensions.empty())
    {
      /* build the regex pattern string.
        For example, if the QStringList contains "xml" and "CFcase" extensions,
        the resulting string will be : "^.+\\.((xml)|(CFcase))$" */

      /// @todo try to use QString::resize() or QString::reserve()
      QString regexPattern;
      std::vector<std::string>::const_iterator it = extensions.begin();

      while(it != extensions.end())
      {
        if(!regexPattern.isEmpty())
          regexPattern.append(")|(");

        regexPattern.append(it->c_str());

        it++;
      }

//      QString regexPattern = extensions.join(")|(");
      regexPattern.prepend("^.+\\.((").append("))$");
      regex.setPattern(regexPattern);
    }
    else
      regex.setPattern("^.+\\..+$");

    while(it != files.end())
    {
      QFileInfo fileInfo = *it;
      QString filename = fileInfo.fileName();

      if (filename != "." && filename != "..")
      {
        if(fileInfo.isDir())
          dirsList.push_back(filename.toStdString());
        else if(includeFiles)
        {
          if(regex.exactMatch(filename))
            filesList.push_back(filename.toStdString());
          else if(includeNoExtension && !filename.contains('.'))
            filesList.push_back(filename.toStdString());
        }
      }

      it++;
    }
  }

  return dirExists;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

                               BOOST SLOTS

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void CCore::read_dir(SignalArgs & args)
{
  SignalFrame & options = args.map( Protocol::Tags::key_options() );

  std::vector<std::string> dirList;
  std::vector<std::string> fileList;
  QString directory;
  rapidxml::xml_attribute<>* attr = args.node.content->first_attribute( "clientid" );
  std::string clientId( attr != nullptr ? attr->value() : "" );

  try
  {
    QString dirPath = options.get_option<std::string>("dirPath").c_str();
    bool includeFiles = options.get_option<bool>("includeFiles");
    bool includeNoExtension = options.get_option<bool>("includeNoExtensions");
    std::vector<std::string> extensions = options.get_array<std::string>("extensions");

    if(dirPath.isEmpty())
      directory = this->DEFAULT_PATH;
    else
      directory = dirPath;

    directory = QDir(directory).absolutePath();
    directory = QDir::cleanPath(directory);

    // if the directory is not the root
    /// @todo test this on Windows
    if(directory != "/")
      dirList.push_back("..");

    if(!this->getDirContent(directory, extensions, includeFiles,
                            includeNoExtension, dirList, fileList))
    {
      m_commServer->sendMessageToClient(dirPath + ": no such direcrory", LogMessage::ERROR, clientId);
    }
    else
    {
      // Build the reply
      SignalFrame reply = args.create_reply( full_path() );
      SignalFrame& roptions = reply.map( Protocol::Tags::key_options() );

      roptions.set_option("dirPath", directory.toStdString());
      roptions.set_array("dirs", dirList, " ; ");
      roptions.set_array("files", fileList, " ; ");
    }
  }
  catch(Exception e)
  {
    CFerror << e.what() << CFflush;
  }

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::newEvent(const std::string & name, const URI & path)
{
  SignalFrame frame(name, path, path);

  m_commServer->sendSignalToClient(*frame.xml_doc.get());
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::createDir(SignalArgs & node)
{
  m_commServer->sendMessageToClient("Cannot create a directory yet", LogMessage::ERROR);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::shutdown(SignalArgs & node)
{
  qApp->exit(0); // exit the Qt event loop
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::saveConfig(SignalArgs & node)
{
  m_commServer->sendMessageToClient("Cannot save the configuration yet", LogMessage::ERROR);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

                                 SLOTS

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void CCore::newClient(const std::string & clientId)
{
  // send a welcome message to the new client
  m_commServer->sendMessageToClient("Welcome to the Client-Server project!", LogMessage::INFO, clientId);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::message(const QString & message)
{
  QString typeStr = message.split(" ").first();
  QString copy(message);
  LogMessage::Type type = LogMessage::Convert::instance().to_enum(typeStr.toStdString());

  m_commServer->sendMessageToClient(copy.remove(0, typeStr.length() + 1), type);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::error(const QString & message)
{
  m_commServer->sendMessageToClient(message, LogMessage::ERROR);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::simulationFinished()
{
  throw NotImplemented(FromHere(), "CCore::simulationFinished");
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::activateSimulation(int clientId, unsigned int nbProcs,
                                      const QString & hosts)
{
  throw NotImplemented(FromHere(), "CCore::activateSimulation");

//  QString error;

//  if(!m_fileOpen)
//    error = "Please open a case file or configure the simulator before activating "
//    "the simulation.";
//  else if(m_simRunning)
//    error = "The simulation is already running.";
//  else if(m_active)
//    error = "The simulation is active.";

//  if(!error.isEmpty())
//  {
//    m_commServer->sendError(clientId, error);
//    m_commServer->sendAck(clientId, false, NETWORK_ACTIVATE_SIMULATION);
//  }
//  else
//  {
//    this->setStatus(WorkerStatus::STARTING);
//    m_simulationManager.spawn("SubSystem", "SubSystem", nbProcs, hosts);
//    m_active = true;
//    m_simulationManager.run();
//  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::deactivateSimulation(int clientId)
{
  throw NotImplemented(FromHere(), "CCore::deactivateSimulation");

//  QString error;

//  if(!m_active)
//    m_commServer->sendError(clientId, "The simulation is not active.");
//  else
//  {
//    this->setStatus(WorkerStatus::EXITING);
//    m_commServer->sendMessage(-1, "Exiting workers.");
//    m_simulationManager.exitWorkers();
//    m_commServer->sendAck(-1, true, NETWORK_DEACTIVATE_SIMULATION);
//    CFinfo << "Simulation has been deativated.\n" << CFflush;
//    m_active = false;
//    this->setStatus(WorkerStatus::NOT_RUNNING);
//  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::spawned()
{
  throw NotImplemented(FromHere(), "CCore::spawned");
}
