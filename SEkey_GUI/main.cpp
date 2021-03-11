/**
  ******************************************************************************
  * File Name          : main.cpp
  * Description        : main of SEkey GUI
  ******************************************************************************
  *
  * Copyright © 2016-present Blu5 Group <https://www.blu5group.com>
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Lesser General Public
  * License as published by the Free Software Foundation; either
  * version 3 of the License, or (at your option) any later version.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  * Lesser General Public License for more details.
  *
  * You should have received a copy of the GNU Lesser General Public
  * License along with this library; if not, see <https://www.gnu.org/licenses/>.
  *
  ******************************************************************************
  */

#include "mainwindow.h"
#include <iostream>
#include <thread>
#include <cstdlib>
#include <fstream>
#include "sekeyclient.h"
#include <QApplication>
#include <QMessageBox>

std::ofstream logfile("GUI_log.txt");

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    sekeyClient client;
    int ret = 0;
    bool connectedToServer = false;
    std::system("kill $(ps -aux | grep \"../../SEkey_backend.exe\" | awk '{print $2}')"); // kill pending backend
    do{
        if(!connectedToServer){ // launch only if not connected
            logger("Start the backend");
            std::thread server = std::thread([]() {
                std::system("../../SEkey_backend.exe");
            });
            server.detach(); // don't care about joining the backend because we will send asynchronous quit message to terminate
            ret = client.sekeyClient_init(); // connet to backend
            if(ret){
                QMessageBox msgBox;
                msgBox.setIcon(QMessageBox::Critical);
                msgBox.setWindowTitle("SEkey");
                msgBox.setText("Fatal error starting the program. Do you want to retry?");
                msgBox.setDetailedText("It is possible that a critical component for running this application is missing.\nCheck if the file SEkey_backend.exe exists in the parent folder of this file.");
                msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
                msgBox.setDefaultButton(QMessageBox::Yes);
                ret = msgBox.exec();
                if(ret != QMessageBox::Yes){ // user decided to abort
                    client.sekeyClient_close();
                    return 0;
                } else { // user decided to go on
                    client.sekeyClient_close(); // close should not really be needed here...just to be careful
                    continue;
                }
            } else {
                connectedToServer = true;
            }
        }
        MainWindow w(nullptr, &client);
        w.show();
        ret = a.exec();
        if(!ret){ // we want to quit the program, so we also close the socket
            client.sekeyClient_close(); // disconnect from backend (ret = 0, MainWindow w is closed)
        } else {
            client.sekeyClient_logout();
        }
    } while(ret);
    return ret;
}
