#include <QtCore>
#include <QtGui>

#include "GUI/Client/NMethod.hpp"

using namespace CF::GUI::Client;

NMethod::NMethod(const QString & name)
  : CNode(name, "CMethod")
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QList<NodeAction> NMethod::getNodeActions() const
{
  static QList<NodeAction> list;

  return list;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QIcon NMethod::getIcon() const
{
  return QFileIconProvider().icon(QFileIconProvider::File);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NMethod::getClassName() const
{
  return "NMethod";
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NMethod::setParams(const QDomNodeList & list)
{

}
