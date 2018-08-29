#include "CursorTextField.h"
#include "MyCharSet.h"

const static float DELTA = 0.5f;

using namespace cocos2d;
using namespace std;

CursorTextField::CursorTextField()
{
	TextFieldTTF();

	m_pCursorSprite = NULL;

}

CursorTextField::~CursorTextField()
{

}

void CursorTextField::onEnter()
{
	TextFieldTTF::onEnter();
	listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = CC_CALLBACK_2(CursorTextField::onTouchBegan, this);
	listener->onTouchEnded = CC_CALLBACK_2(CursorTextField::onTouchEnded, this);
	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);
	this->setDelegate(this);
}

CursorTextField * CursorTextField::textFieldWithPlaceHolder(Node * node, const char *placeholder, const char *fontName, float fontSize)
{
	auto pRet = new CursorTextField();
	pRet->parentNode = node;
	if (pRet && ((TextFieldTTF*)pRet)->initWithPlaceHolder(placeholder, fontName, fontSize))
	{
		pRet->autorelease();
		if (placeholder)
		{
			pRet->setPlaceHolder(placeholder);
		}
		pRet->init();
		pRet->initCursorSprite(fontSize);
		pRet->setHorizontalAlignment(kCCTextAlignmentLeft);
		return pRet;
	}
	CC_SAFE_DELETE(pRet);
	return NULL;
}

bool CursorTextField::init() {
	this->inputFrameWidth = 400;
	this->inputMaxLength = 16;
	this->nLenCount = 0;
	memset(codeNumType, 0, sizeof(codeNumType));
	this->codeCur = 0;
	this->startCur = 0;
	this->endCur = 0;
	inpuText = "";
	return true;
}
void CursorTextField::initCursorSprite(const int mHeight)
{
	int column = 2;
	int nHeight = 20;
	int pixels[50][2];
	for (int i = 0; i < nHeight; ++i)
	{
		for (int j = 0; j < column; ++j)
		{
			pixels[i][j] = 0xffffffff;
		}
	}
	Texture2D* texture = new Texture2D();
	texture->initWithData(pixels, 20, Texture2D::PixelFormat::RGB888, 4, mHeight, CCSizeMake(column, nHeight));
	m_pCursorSprite = Sprite::createWithTexture(texture);
	texture->autorelease();
	if (m_pCursorSprite == nullptr)
	{
		CCLOG("NULL");
	}
	Size winSize = getContentSize();
	m_pCursorSprite->setVisible(false);
	parentNode->addChild(m_pCursorSprite);
	m_pCursorAction = RepeatForever::create(Sequence::create(FadeOut::create(0.25f), FadeIn::create(0.25f), NULL));
	m_pCursorSprite->runAction(m_pCursorAction);
}

void CursorTextField::setPosition(float &x, float &y)
{
	Point posi(x, y);
	setPosition(posi);
}

void CursorTextField::setPosition(const Point& pos)
{
	TextFieldTTF::setPosition(pos);
	// 设置光标位置
	if (NULL != m_pCursorSprite)
	{
		Size winSize = getContentSize();
		m_cursorPos = ccp(0, 0/*winSize.height / 2*/);
		m_cursorPos = m_cursorPos + pos;
		m_pCursorSprite->setPosition(m_cursorPos.x, m_cursorPos.y + m_pCursorSprite->getContentSize().height / 2.0);
	}
}


void CursorTextField::setCursorPositionX(float x) // 设置光标x位置
{
	Point pt = getPosition(); // 获取输入框位置
	m_pCursorSprite->setPositionX(pt.x + x);
}
// 把光标添加到和输入框一起的层中
void CursorTextField::AddCursor(Node *node)
{
	if (NULL != node && NULL != m_pCursorSprite)
	{
		node->addChild(m_pCursorSprite);
		m_pCursorSprite->setPositionY(getContentSize().height / 2.0);
		m_pCursorSprite->runAction(m_pCursorAction);
	}
}

bool CursorTextField::onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent)
{
	m_beginPos = pTouch->getLocation();
	return true;
}

Rect CursorTextField::getRect()
{
	Size size = getContentSize();
	return  CCRectMake(0, -size.height / 2, inputFrameWidth, size.height);
}

//获取输入框内容
const char* CursorTextField::getInputText() {
	const char* text = m_pInputText.c_str();
	return text;
}

//设置输入框内容
void CursorTextField::setInpuntText(char* text) {
	m_pInputText = "";
	setString(text);
	m_pCursorSprite->setPositionX(this->getPosition().x);
	memset(codeNumType, 0, sizeof(codeNumType));
	codeCur = 0;
	startCur = 0;
	endCur = 0;
	inpuText = "";
}

//设置输入框宽度 一旦字符串宽度超度这个长度 字符串会自动向左缩进
void CursorTextField::setInputWidth(float width) {
	this->inputFrameWidth = width;
}

//设置输入宽显示的最大字符数量Unicode
void CursorTextField::setInputMaxLength(float length) {
	this->inputMaxLength = length;
}

//判断点击事件，是否响应在输入框范围内
bool CursorTextField::isInTextField(cocos2d::Touch *pTouch)
{
	return getRect().containsPoint(convertTouchToNodeSpaceAR(pTouch));
}

void CursorTextField::onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent)
{
	Point endPos = pTouch->getLocation();
	// 判断是否为点击事件
	if (::abs(endPos.x - m_beginPos.x) > DELTA ||
		::abs(endPos.y - m_beginPos.y))
	{
		// 不是点击事件
		m_beginPos.x = m_beginPos.y = -1;
		return;
	}

	// 判断是打开输入法还是关闭输入法
	isInTextField(pTouch) ? openIME() : closeIME();
}

//弹出手机键盘时响应事件
bool CursorTextField::onTextFieldAttachWithIME(cocos2d::TextFieldTTF *pSender)
{
	if (m_pInputText.empty()) {
		return false;
	}
	m_pCursorSprite->setPositionX(this->getPosition().x + getContentSize().width);
	return false;
}

//当有输入进来时响应
//@param pSender 发送事件对象
//@param text 输入内容
//@param  内容字节长度


bool CursorTextField::onTextFieldInsertText(cocos2d::TextFieldTTF* sender, const char * text, size_t nLen)
{
	int j = 0;
	std::string mytext = text;
	for (j = 0; j < mytext.size();)
	{
		std::string sText = m_pInputText.c_str();
		wchar_t* wText = new wchar_t[200];
		char t[200];
		memset(t, 0, sizeof(t));
		strcpy(t, sText.c_str());
		int unisize = 0;
		int cou = MyCharSet::getinstence()->utf8_to_unicode((uint8_t*)t, (uint16_t **)wText, &unisize);
		std::string ss = split_text(mytext, 1, j);
		j += ss.length();
		//当字符数量超过规定值 不做处理
		if (cou >= inputMaxLength)
		{
			CC_SAFE_DELETE_ARRAY(wText);
			return true;
		}
		//return true;
		//屏蔽回车输入
		if (ss == "\n")
		{
			CC_SAFE_DELETE_ARRAY(wText);
			continue;
		}
		//输入框总内容添加
		m_pInputText.append(ss);


		//输入框当前字符串添加
		inpuText.append(ss);
		//当前字符的长度
		codeNumType[codeCur++] = ss.length();
		std::string localText = m_pInputText;
		setString(m_pInputText);
		//如果总字符串的长度大于指定宽度
		if (getContentSize().width > inputFrameWidth) {
			//大于，截取字符串，直到字符串的长度小于指定宽度为止
			setString(inpuText);
			while (getContentSize().width > inputFrameWidth) {
				int nnLen = ss.length();
				if (codeNumType[startCur] == 1) {
					nnLen = 1;
				}
				if (codeNumType[startCur] == 3) {
					nnLen = 3;
				}
				startCur++;
				nLenCount += nnLen;
				float gap = localText.size() - nLenCount;
				inpuText = localText.substr(nLenCount, gap);
				setString(inpuText.c_str());
				float coWidth = getContentSize().width;
			}
		}
		else {
			//小于，直接设置显示总字符串
			nLenCount = 0;
			startCur = 0;
			setString(m_pInputText);
		}
		//设置光标位置
		m_pCursorSprite->setPositionX(this->getPosition().x + getContentSize().width);
		CC_SAFE_DELETE_ARRAY(wText);
		//AndroidShowtext(mychar, 1);
	}
	return true;
}


//当有输入进来时响应
//@param pSender 发送事件对象
//@param text 删除内容
//@param  内容字节长度

bool CursorTextField::onTextFieldDeleteBackward(cocos2d::TextFieldTTF* sender, const char * delText, size_t nLen)
{
	// 将总字符串长度减去nLen字节长
	m_pInputText.resize(m_pInputText.size() - nLen);
	//当前字符数减一
	codeNumType[codeCur--] = 0;
	std::string localText = m_pInputText;
	setString(m_pInputText);
	if (getContentSize().width > inputFrameWidth) {
		//大于指定宽度，截取字符串，直到字符串长度小于指定宽度
		while (getContentSize().width > inputFrameWidth) {
			int nnLen = nLen;
			if (codeNumType[startCur - 1] == 1) {
				nnLen = 1;
			}
			if (codeNumType[startCur - 1] == 3) {
				nnLen = 3;
			}
			nLenCount -= nnLen;
			startCur--;
			if (startCur <= 0)
				startCur = 0;
			if (nLenCount <= 0)
				nLenCount = 0;
			float gap = localText.size() - nLenCount;
			const std::string text = localText.substr(nLenCount, gap);
			setString(text);
			inpuText = text;
		}
	}
	else {
		nLenCount = 0;
		startCur = 0;
		setString(m_pInputText.c_str());
	}
	//设置光标位置
	m_pCursorSprite->setPositionX(this->getPosition().x + getContentSize().width);
	if (m_pInputText.empty()) {
		m_pCursorSprite->setPositionX(this->getPosition().x);
	}
	return true;
}

bool CursorTextField::onTextFieldDetachWithIME(cocos2d::TextFieldTTF *pSender)
{
	return false;
}

void CursorTextField::openIME()
{
	m_pCursorSprite->setVisible(true);
	setString(m_pInputText);
	((TextFieldTTF *)this)->attachWithIME();
}

void CursorTextField::closeIME()
{
	m_pCursorSprite->setVisible(false);
	//auto pTextField = (TextFieldTTF *)pRet;
	((TextFieldTTF *)this)->detachWithIME();
}

void CursorTextField::onExit()
{
	TextFieldTTF::onExit();
	Director::getInstance()->getEventDispatcher()->removeEventListener(listener);
}

std::string CursorTextField::split_text(std::string name, int len, int start)
{
	int i = start;
	std::string str;
	if (name[i] < 0)
	{
		i++;
	}
	//while (name[i] < 0)
	//{
	//  i++;
	//  if (i - start == 2)
	//  {
	//      break;
	//  }
	//}
	if (start == i)
	{
		str = name.substr(start, 1);
	}
	else
	{
		str = name.substr(start, 3);
	}
	//log("mysubstr   %s", str.c_str());
	return str;
}