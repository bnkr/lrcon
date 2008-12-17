#ifndef SERVERMANAGER_HPP_sdfsdf
#define SERVERMANAGER_HPP_sdfsdf

#include "common.hpp"

#include <lrcon/rcon.hpp>

#include <QMessageBox>
#include <QObject>
#include <QLineEdit>
#include <QTextEdit>

#include <exception>

/*!

\todo Make this into a propah mainwindow type thing.  I think this is the best way
      to implement a lot of the features I was looking for.  There's not much of a
      best practices guide tho.

*/

class ServerManager : public QObject {
  Q_OBJECT
  public:
    const QLineEdit &host_;
    const QLineEdit &port_;
    const QLineEdit &password_;
    const QLineEdit &command_;
    QTextEdit &output_;
    
    ServerManager(const QLineEdit &host, const QLineEdit &port, 
                  const QLineEdit &password, const QLineEdit &command, QTextEdit &output)
    : host_(host), port_(port), password_(password), command_(command), output_(output) { }
    
  public slots:
    void commandEntered();
    
  signals:
    void connected();
    
  private:
    void generalError(const char *header, const char *text, QMessageBox::Icon i = QMessageBox::Warning); 
};



#endif 
