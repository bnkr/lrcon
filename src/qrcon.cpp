#include <lrcon/rcon.hpp>

#include <QObject>
#include <QApplication>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFontMetrics>
 
#include <memory>

/**

\todo
- signal() will calls the save state routine (somehow)
- save custom servers --- maybe replace the current inputs and make them always be saved
- automatically focus to server details if they were not loaded; else the comand box
- Extension: save server,port,password (encrypt it somehow)
  List of recent servers,port,password (dropdown completion from the server input box 
  I hope... could be hard due to multiple server/ports tho)
- save and subsequently reload window size
- onclick of the server attrs, the whole thing is selected
- it seems that the autotroll thingymajig doesn't work properly... but the example generates it 
  ok... ???  It won't run MOC... maybe because I have .hpp aargh.  Also it seems it only does
  it for make dist... ???  Not really what I wanted
- up arrow on the command box shows previous commands
- proper validation of inputs
- handling of errors from the protocol
- 

*/



#include "ServerManager.hpp"



int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  
//   std::auto_ptr<QPushButton> button(new QPushButton("Quit"));
  std::auto_ptr<QWidget> main_window(new QWidget());
  main_window->setWindowTitle("qRCON");
  /// non-portable.  I want a) to remember and subsequently reload the size, and b) to make the 
  /// default size a percentage of the resolution
  main_window->resize(500, 600); 
  
  QLabel *server_name_label(new QLabel("<b>Server Name:</b>", main_window.get()));
  QLineEdit *server_name(new QLineEdit(main_window.get()));
  QLabel *server_port_label(new QLabel("<b>Port:</b>", main_window.get()));
  QLineEdit *server_port(new QLineEdit(main_window.get()));
  QLabel *server_password_label(new QLabel("<b>Password:</b>", main_window.get()));
  QLineEdit *server_password(new QLineEdit(main_window.get()));
  QFontMetrics f = server_password->fontMetrics();
  server_password->setMaxLength(rcon::command::max_string_length-1);
  f = server_password->fontMetrics();
  QSize s(server_password->minimumSizeHint());
  s.setWidth(f.width('0')*15); /// <-- not exact at all
  server_password->setMaximumSize(s);
  
  server_port->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
  //server_port->setInputMask("00000"); /// <-- this makes it do stupid things
  f = server_port->fontMetrics();
  server_port->setMaxLength(5);
  s = server_port->minimumSizeHint();
  s.setWidth(f.width('0')*7); /// <-- not exact at all (7 is about the right width for 5 0s.... )
  server_port->setMaximumSize(s);
  

  std::auto_ptr<QHBoxLayout> server_details(new QHBoxLayout());
  server_details->addWidget(server_name_label);
  server_details->addWidget(server_name);
  server_details->addWidget(server_port_label);
  server_details->addWidget(server_port);
  server_details->addWidget(server_password_label);
  server_details->addWidget(server_password);
  
  QTextEdit *server_output(new QTextEdit(main_window.get()));
  server_output->setReadOnly(true);
  
  QLabel *send_command_label(new QLabel("<b>Command:</b>", main_window.get()));
  QLineEdit *command_input(new QLineEdit(main_window.get()));
  QPushButton *send_button(new QPushButton("Send", main_window.get()));
  QPushButton *quit_button(new QPushButton("Quit", main_window.get()));
  
  QHBoxLayout *send_panel(new QHBoxLayout());
  send_panel->addWidget(send_command_label);
  send_panel->addWidget(command_input);
  send_panel->addWidget(send_button);
  send_panel->addWidget(quit_button);
  
  QGridLayout *main_layout(new QGridLayout());
  main_layout->addLayout(server_details.get(), 0, 0);
  main_layout->addWidget(server_output, 1, 0);
  main_layout->addLayout(send_panel, 2, 0);
  
  ServerManager mgr(*server_name, *server_port, *server_password, *command_input, *server_output);

  QObject::connect(send_button, SIGNAL(clicked()),
                   &mgr, SLOT(commandEntered()));
  
  QObject::connect(command_input, SIGNAL(returnPressed()),
                   &mgr, SLOT(commandEntered()));
  
  QObject::connect(quit_button, SIGNAL(clicked()),
                   &app, SLOT(quit()));
  
  main_window->dumpObjectTree();
  
  main_window->setLayout(main_layout);
  main_window->show();
  
  return app.exec();
}
