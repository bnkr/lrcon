#include "ServerManager.hpp" 

#include <QString>

void ServerManager::commandEntered() {
  /// \todo this should only be declared once actually, but what
  ///       if it kicks you off?  Is there some timout?  Are you
  ///       authed forever-- what!?!
  QRCON_DEBUG_MESSAGE("initialising");
  rcon::connection conn(rcon::host(host_.text().toAscii().constData(), port_.text().toAscii().constData()), 
                        password_.text().toAscii().constData());
  
  
  
  /// \todo deal with connection failure here
  
  emit connected();
  output_.append(QString("<b>&gt; ") + command_.text() + "</b>");
  
  QRCON_DEBUG_MESSAGE("sending " << command_.text().toAscii().constData());
  
  rcon::command cmd(conn, command_.text().toAscii().constData());
  
  /// \todo deal with command failure here
  
  output_.append(QString(cmd.data().c_str()));
}
