// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_Log_hpp
#define CF_Common_Log_hpp

#include "Common/CommonAPI.hpp"
#include "Common/LogLevel.hpp"
#include "Common/LogStream.hpp"
#include "Common/CodeLocation.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

class LogStream;

/// @brief Main class of the logging system.

/// This class manages all streams used for logs. This class is a singleton. @n
/// @c Logger manages 4 streams: @e info, @e error, @e warning, @e debug.
/// These streams differ on the type of messages they forward. By
/// default, only streams @c #ERROR, @c #WARNING, @c #INFO will be outputted.
/// This can be changed using Logger::set_log_level().
/// By default no file is open.
/// Once an MPI environment has been initialized, @c #openFiles() can be called.
/// Two files descriptors are created: one for a @e log file and another one
/// for a @e trace file; respectively <code>output-p<i>i</i>.log</code> and
/// <code>output-p<i>i</i>.trace</code>, where <code><i>i</i></code> if the
/// MPI rank number).

/// @see LogStream
/// @author Quentin Gasper
class Common_API Logger : public boost::noncopyable {

  public:

  /// @brief Gives the current @c #Logger instance.

  /// If no instance exists, a new one is created.
  /// @return Returns the current instance.
  static Logger& instance();

  /// @brief Initiates the log environment base on the CEnv component
  /// maintained by the Core.
  void initiate();

  /// @returns the class name
  static std::string type_name() { return  "Logger"; }

  /// @brief Gives the info stream.
  /// @param place The code location from where this method was called
  /// @return Returns a reference to the info stream.
  LogStream & Info (const CodeLocation & place);

  /// @brief Gives the error stream.
  /// @param place The code location from where this method was called
  /// @return Returns a reference to the error stream.
  LogStream & Error (const CodeLocation & place);

  /// @brief Gives the warning stream.
  /// @param place The code location from where this method was called
  /// @return Returns a reference to the warning stream.
  LogStream & Warn (const CodeLocation & place);

  /// @brief Gives the debug stream.
  /// @param place The code location from where this method was called
  /// @return Returns a reference to the debug stream.
  LogStream & Debug (const CodeLocation & place);


  LogStream & getStream(LogLevel type);

  /// @brief Creates file descriptors and gives them to streams.
  void openFiles();

  void set_log_level(const Uint log_level);

  private :

  /// @brief Managed streams.

  /// The key is the stream type. The value is a pointer to the stream.
  std::map<LogLevel, LogStream *> m_streams;

  /// @brief Constructor
  Logger();

  /// @brief Destructor
  ~Logger();

}; // class Logger

////////////////////////////////////////////////////////////////////////////////
// Logging macros
////////////////////////////////////////////////////////////////////////////////

/// these are always defined

#define CFinfo      CF::Common::Logger::instance().Info (FromHere())
#define CFerror     CF::Common::Logger::instance().Error(FromHere())
#define CFwarn      CF::Common::Logger::instance().Warn (FromHere())
#define CFdebug     CF::Common::Logger::instance().Debug(FromHere())
#define CFflush     CF::Common::LogStream::ENDLINE
#define CFendl      '\n' << CFflush

#define CFLog(n,x)     CFinfo  << n << x << CFflush;
#define CFLogInfo(x)   CFinfo  << x << CFflush;
#define CFLogWarn(x)   CFwarn  << x << CFflush;
#define CFLogError(x)  CFerror << x << CFflush;

////////////////////////////////////////////////////////////////////////////////
// Debugging macros
////////////////////////////////////////////////////////////////////////////////

#ifndef CF_NO_DEBUG_MACROS

/// log the value of a variable
#define CFLogVar(x) CFinfo << #x << " = " << x << CFendl;
/// Definition of a macro for placing a debug point in the code
#define CF_DEBUG_POINT  CFdebug << "DEBUG : " << __FILE__ << " : " << __LINE__ << " : " << __FUNCTION__ << "\n" ; CFdebug.flush()
/// Definition of a macro for outputing an object that implements the output stream operator
#define CF_DEBUG_OBJ(x) CFdebug << "DEBUG : OBJECT " << #x << " -> " << x << " : " << __FILE__ << " : " << __LINE__ << " : " << __FUNCTION__ << "\n" ; CFdebug.flush()
/// Definition of a macro for outputing a debug string in the code
#define CF_DEBUG_STR(x) CFdebug << "DEBUG : STRING : " << x << " : " << __FILE__ << " : " << __LINE__ << " : " << __FUNCTION__ << "\n" ; CFdebug.flush()
/// Definition of a macro for debug abort
#define CF_DEBUG_ABORT  CFdebug << "DEBUG : ABORT " << __FILE__ << " : " << __LINE__ << " : " << __FUNCTION__ << "\n" ; CFdebug.flush() ; abort()

#else

#define CF_DEBUG_POINT
#define CF_DEBUG_OBJ(x)
#define CF_DEBUG_STR(x)
#define CF_DEBUG_ABORT

#endif // CF_NO_DEBUG_MACROS

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_Log.hpp
