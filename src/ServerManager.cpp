#include "ServerManager.hpp" 

#include <QString>

void ServerManager::commandEntered() {
  if (command_.text().length() == 0) {
    return;
  }
  
  if (port_.text().length() == 0 || host_.text().length() == 0) {
    generalError("Incomplete server details.", "Please give the server name and port number.");
    return;
  }

  
  try {
    QRCON_DEBUG_MESSAGE("Open connection.");
    
    const char *password = (password_.text().length()) 
        ? password_.text().toAscii().constData()
        : "";
    
    /// \todo this should only be declared once actually, but what
    ///       if it kicks you off?  Is there some timout?  Are you
    ///       authed forever-- what!?!  I think I will need a
    ///       reconnect() method.  Implies we should store the 
    ///       data from those fields tho.
    rcon::connection conn(rcon::host(host_.text().toAscii().constData(), port_.text().toAscii().constData()), 
                          password);
  
    emit connected();
    output_.append(QString("<b>&gt; ") + command_.text() + "</b>");
    
    QRCON_DEBUG_MESSAGE("sending " << command_.text().toAscii().constData());
  
    rcon::command cmd(conn, command_.text().toAscii().constData());
  
    output_.append(QString(cmd.data().c_str()));
  }
  /// \todo better exception handling
  catch (std::exception &e) {
    QRCON_DEBUG_MESSAGE("handling exception");
    generalError("The command returned an error.", e.what());
  }
  catch (...) {
    generalError("An unknown exception occured.", 
                 "An unhandled exception occured when dealing with the command.  This is a program error.",
                 QMessageBox::Critical
                );
  }
}

void ServerManager::generalError(const char *header, const char *text, QMessageBox::Icon icon) {
  QMessageBox msg_box;
  msg_box.setIcon(icon);
  msg_box.setText(header);
  msg_box.setInformativeText(text);
  msg_box.exec();
}
