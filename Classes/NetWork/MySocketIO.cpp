#include "MySocketIO.h"

Scene* MySocketIO::createScene()
{
	Scene* s = Scene::create();
	MySocketIO* l = MySocketIO::create();
	s->addChild(l);
	return s;
}

bool MySocketIO::init() 
{
	if (!Layer::init()) 
	{
		return false;
	}

	_socketlient = nullptr;

	auto menu = Menu::create();
	this->addChild(menu);

	auto lblInit = Label::create("Init socket", "Arial", 22);

	auto menuInit = MenuItemLabel::create(lblInit, [=](Ref*) 
	{
		if (!_socketlient) 
		{
			_socketlient = SocketIO::connect("127.0.0.1:8002", *this);
			if (_socketlient) 
			{
				_socketlient->setTag("init socket");
				_socketlient->on("textevent", [=](SIOClient* c, const std::string& data)
				{
					log("textSocketd::textevent called with data %s ", data.c_str());
				});
				_socketlient->on("event", [=](SIOClient* c, const std::string& data)
				{
					log("textSocked::event called with data %s", data.c_str());
				});
			}
		}
	});

	auto lblSend = Label::create("send message", "Arial", 22);

	auto menuSend = MenuItemLabel::create(lblSend, [=](Ref*)
	{
		if (_socketlient)
		{
			//_socketlient->send("");
		}
	});

	menu->addChild(menuInit);
	menu->addChild(menuSend);

	menu->alignItemsVertically();

	return true;
	
}

void MySocketIO::sendEvent()
{
	if (_socketlient) 
	{
		_socketlient->emit("event", "[{\"name:\"myname\",\"type\":\"mytype\"}]");
	}
}

void MySocketIO::closeSokcet()
{
	if (_socketlient) 
	{
		_socketlient->disconnect();
	}
}

void MySocketIO::onConnect(SIOClient* client)
{
	log("onConnect");
	log("%s connect", client->getTag());
}

void MySocketIO::onMessage(SIOClient* client, const std::string& data)
{
	log("onMessage");
	log("%s received content is:%s ", client->getTag(), data.c_str());
}

void MySocketIO::onClose(SIOClient* client)
{
	log("onClose");
	log("%s received content is :%s", client->getTag());
}

void MySocketIO::onError(SIOClient* client, const std::string& data)
{
	log("error");
	log("%s is Error %s", client->getTag(), data.c_str());
}

