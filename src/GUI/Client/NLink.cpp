#include <QtCore>
#include <QtGui>

#include "Common/CF.hpp"

#include "GUI/Client/NLink.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;

NLink::NLink(const QString & name)
  : CNode(name, "CLink")
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QList<NodeAction> NLink::getNodeActions() const
{
  static QList<NodeAction> list;

  return list;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QIcon NLink::getIcon() const
{
  return QFileIconProvider().icon(QFileIconProvider::Network);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NLink::getClassName() const
{
  return "NLink";
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NLink::setParams(const QDomNodeList & list)
{
  throw NotImplemented(FromHere(), "NLink::setParams");
}
