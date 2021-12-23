#include "websocket/wsserver.h"
#include <QMessageBox>
#include <QList>

WsServer::WsServer(QTreeWidgetItem *qTreeWidgetItemServer, QGridLayout *qGridLayoutParent, QString ip, quint16 port)
{
    this->qTreeWidgetItemServer = qTreeWidgetItemServer;
    this->qGridLayoutParent = qGridLayoutParent;
    this->ip = ip;
    this->port = port;
}

WsServer::~WsServer(){
    QHashIterator<long,WsServerConnection*> iter(serverConnectionList);
    QList<long> qList;
    while(iter.hasNext())
    {
        iter.next();
        qList.push_back(iter.key());
    }

    for(int i=0;i<qList.size();i++){
        WsServerConnection *item=serverConnectionList.value(qList.value(i));
        serverConnectionList.remove(qList.value(i));
        delete item;
        item=nullptr;
    }
    if(qTreeWidgetItemServer!=nullptr){
        delete qTreeWidgetItemServer;
        qTreeWidgetItemServer=nullptr;
    }
    if(qVBoxLayoutLeft!=nullptr){
        delete qVBoxLayoutLeft;
        qVBoxLayoutLeft=nullptr;
    }
    if(server!=nullptr){
        server->deleteLater();
        server=nullptr;
    }
}

bool WsServer::start(){
    server = new QWebSocketServer(QStringLiteral("Server"), QWebSocketServer::NonSecureMode, this);
    QHostAddress address = QHostAddress(ip);
    if(!server->listen(address, port)) {
         QMessageBox::information(nullptr,tr("Error"),server->errorString());
         return false;
    }
    connect(server,SIGNAL(newConnection()),this,SLOT(connection_connected()));//Listening 
    return true;
}

void WsServer::connection_connected() {
    QWebSocket *qWebSocket = server->nextPendingConnection();

    QTreeWidgetItem *qTreeWidgetItemConnection=new QTreeWidgetItem();
    qTreeWidgetItemConnection->setText(0,qWebSocket->peerAddress().toString()+":"+QString::number(qWebSocket->peerPort()));
//    ServerConnection *serverConnection=new ServerConnection(qTreeWidgetItemConnection,qGridLayoutParent,qTcpSocket,&deleteServerConnectionThread->qMutex,&deleteServerConnectionThread->q,&deleteServerConnectionThread->qSemaphore);
    WsServerConnection *serverConnection=new WsServerConnection(qTreeWidgetItemConnection,qGridLayoutParent,qWebSocket,&serverConnectionList,qWebSocket->peerAddress().toString(),qWebSocket->peerPort());
    serverConnection->pingCheckBox->setEnabled(true);
    long key=reinterpret_cast<long>(qTreeWidgetItemConnection);
    serverConnectionList.insert(key,serverConnection);
//    qDebug()<<&serverConnectionList;
    qTreeWidgetItemServer->addChild(qTreeWidgetItemConnection);
}

void WsServer::deleteServerConnection(long key){
    WsServerConnection *serverConnection=serverConnectionList.value(key);
    serverConnectionList.remove(key);
    delete serverConnection;
}
