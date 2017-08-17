///////////����������������������������������������������������������������������������������������/�����б���
////�����б�
#ifndef DROPDOWNLIST_h
#define DROPDOWNLIST_h
#include "cocos2d.h"
#include "ui/CocosGUI.h"
USING_NS_CC;




class DropDownList : public Layer
{
public:
	//���췽��
	DropDownList(LabelTTF* label, Size size, int scrollInnerHeight)
		: showLabel(label)
		, isShowMenu(false)
		, lastSelectedIndex(0)
	{
		isTouchBegin = false;

		// ����һ��scrollView , �������
		Size scrollViewSize = Size(size.width, 700);

		float scrollInnerContainHeight = scrollInnerHeight * size.height;

		scrollView = ui::ScrollView::create();
		scrollView->setBounceEnabled(true);
		scrollView->setSwallowTouches(true);
		scrollView->setContentSize(scrollViewSize);  // ���ô�������Ĵ�С
		scrollView->setAnchorPoint(Point(0.5, 1));
		scrollView->setPosition(Point(size.width / 2, 0));
		scrollView->setInnerContainerSize(Size(scrollViewSize.width, scrollInnerContainHeight));   //�����ڲ��������Ĵ�С
		this->addChild(scrollView);
		scrollView->setVisible(false);

		// ��������ı���
		/*Sprite* bg = Sprite::create("scrollViewBox.png");
		bg->setScaleX(scrollViewSize.width / bg->getContentSize().width);
		bg->setScaleY(scrollViewSize.height / bg->getContentSize().height);
		bg->setPosition(scrollViewSize.width / 2, scrollViewSize.height/2);
		scrollView->addChild(bg , -1000);*/

		//������һ��menu���������ڻ�����ӵ���ͼ��
		//mainMenu = Menu::create();
		//mainMenu->setPosition(Point(size.width / 2, size.height / 2));
		//mainMenu->retain();

		showLabel->setPosition(Point(size.width / 2, size.height / 2));
		showLabel->setColor(Color3B(200,90,40));
		addChild(showLabel);
		////
		selectBg = Sprite::create("listBg1.png");
		selectBg->setPosition(Point(size.width / 2, size.height / 2));
		selectBg->setScaleX(size.width / selectBg->getContentSize().width);
		selectBg->setScaleY(size.height / selectBg->getContentSize().height);
		selectBg->setOpacity(180);
		addChild(selectBg, -100);

		setContentSize(size);

		m_listId = ++DropDownList::listId;
	}

	//��������
	~DropDownList()
	{
		//mainMenu->release();

	}

	//����ʵ�����󷽷�
	static DropDownList* create(LabelTTF* label, Size size, int scrollInnerHeight)
	{
		DropDownList* list = new DropDownList(label, size, scrollInnerHeight);
		list->autorelease();
		return list;
	}
	//��ȡ��ǰѡ��label��string
	std::string getString()
	{
		return showLabel->getString();
	}
	void setString(std::string str)
	{
		showLabel->setString(str);
	}

	//��ȡ��ǰѡ�е�index
	int getSelectedIndex()
	{
		return lastSelectedIndex;
	}

	//����ѡ��index
	void setSelectedIndex(int index)
	{
		lastSelectedIndex = index;

		for (int i = 0, j = (int)selectLabels.size(); i < j; ++i)
		{
			if (i == lastSelectedIndex)
			{
				bgSprites[i]->setColor(Color3B(255, 40, 255));
				showLabel->setString(selectLabels[i]->getString());
				showLabel->setScaleX(selectLabels[i]->getScaleX());

				
			}
			else
			{
				bgSprites[i]->setColor(Color3B(255, 255, 255));
			}
		}
	}

	////����ѡ������ĸ������֣��������ֻ��
	void setSelectedIndexByString(std::string name)
	{
		for (int i = 0; i < (int)selectLabels.size(); ++i)
		{
			if (name == selectLabels[i]->getString()) {
				bgSprites[i]->setColor(Color3B(255, 40, 255));
				showLabel->setString(selectLabels[i]->getString());
				showLabel->setScaleX(selectLabels[i]->getScaleX());

				lastSelectedIndex = i;

				//scrollView->scrollToPercentVertical( float(i) / selectLabels.size() * 100.0 , 1 , false);
				
			}
			else
			{
				bgSprites[i]->setColor(Color3B(255, 255, 255));
			}
		}
	}

	void onEnter()
	{
		setTouchEnabled(true);
		auto listener = EventListenerTouchOneByOne::create();
		listener->setEnabled(true);
		listener->onTouchBegan = [=](Touch *touch, Event *event)
		{
			Rect rect;
			rect.origin = Vec2(0, 0);
			rect.size = getContentSize();
			Point position = convertTouchToNodeSpace(touch);

			if (rect.containsPoint(position))
			{
				isTouchBegin = true;
			}

			if (DropDownList::isShowOneList == true) {
				if (DropDownList::nowShowListId == this->m_listId) {
					
					return true;
				}
				else {
					return false;
				}
			}
			else {
				return true;
			}
			return true;
		};
		listener->onTouchEnded = [=](Touch *touch, Event *event) {
			if (!isShowMenu)
			{
				Rect rect;
				rect.origin = Vec2(0, 0);
				rect.size = getContentSize();
				Point position = convertTouchToNodeSpace(touch);

				if (rect.containsPoint(position) && isTouchBegin)
				{
					isShowMenu = true;
					DropDownList::isShowOneList = true;
					DropDownList::nowShowListId = this->m_listId;

					//�������ʾ�����б��Ҳ����mainMenu
					//addChild(mainMenu);
					scrollView->setVisible(true);

					if (scrollView->getContentSize().height <= scrollView->getInnerContainerSize().height) {
						scrollView->scrollToPercentVertical(float(lastSelectedIndex) / selectLabels.size() * 100.0, 0.6, false);
						//CCLOG("1 scrollView ,ContentSize HEIGHT:%f , InnerContainerSize , height : %f ", scrollView->getContentSize().height, scrollView->getInnerContainerSize().height);
					}
					else {
						//CCLOG("2 scrollView ,ContentSize HEIGHT:%f , InnerContainerSize , height : %f ", scrollView->getContentSize().height, scrollView->getInnerContainerSize().height);
					}
					

					for (int i = 0, j = (int)selectLabels.size(); i < j; ++i)
					{
						if (i == lastSelectedIndex)
						{
							bgSprites[i]->setColor(Color3B(255, 40, 255));
						}
						else
						{
							bgSprites[i]->setColor(Color3B(255, 255, 255));
						}
					}
					//return true;
				}
			}
			else {
				onClose();
				//return true;
			}

			isTouchBegin = false;
			//return false;
		};

		EventDispatcher *dispatcher = Director::getInstance()->getEventDispatcher();
		dispatcher->addEventListenerWithSceneGraphPriority(listener, this);

		Layer::onEnter();
	}

	void addLabelDynamic(LabelTTF* label)
	{
		
		////����������м���һ����ť
		Size size = getContentSize();
		
		Size innerSize = scrollView->getInnerContainerSize();
		scrollView->setInnerContainerSize(Size(innerSize.width, innerSize.height + size.height));

		if (innerSize.height < scrollView->getContentSize().height) {
			innerSize.height = scrollView->getContentSize().height;
		}
		else {
			for (int i = 0; i < bgSprites.size(); ++i) {
				bgSprites[i]->setPositionY(bgSprites[i]->getPositionY() + size.height);
			}
		}


		selectLabels.push_back(label);

		//float scrollInnerContainHeight = selectLabels.size() * size.height;
		//scrollView->setInnerContainerSize(Size(size.width, scrollInnerContainHeight));

		ui::Button* button = ui::Button::create("listBg2.png", "listBg3.png");
		float sx = size.width / button->getContentSize().width;
		float sy = size.height / button->getContentSize().height;
		button->setScaleX(sx);
		button->setScaleY(sy);
		button->setAnchorPoint(Point(0, 0));
		button->setPosition(Point(0, scrollView->getInnerContainerSize().height - selectLabels.size() * size.height));
		label->setAnchorPoint(Point(0, 0.5));
		label->setPosition(Point(0, size.height / 2));
		label->setColor(Color3B(250, 160, 30));
		button->addChild(label);

		bgSprites.push_back(button);
		button->setTag((int)selectLabels.size() - 1);
		button->setSwallowTouches(true);
		button->setOpacity(250);
		scrollView->addChild(button);

		button->addTouchEventListener(callfunc);
	}

	//������menu item �����һ��label���ǵ�����
	void addLabel(LabelTTF* label)
	{

		////����������м���һ����ť
		Size size = getContentSize();
		selectLabels.push_back(label);

		//float scrollInnerContainHeight = selectLabels.size() * size.height;
		//scrollView->setInnerContainerSize(Size(size.width, scrollInnerContainHeight));

		ui::Button* button = ui::Button::create("listBg2.png", "listBg3.png");
		float sx = size.width / button->getContentSize().width;
		float sy = size.height / button->getContentSize().height;
		button->setScaleX(sx);
		button->setScaleY(sy);
		button->setAnchorPoint(Point(0, 0));
		button->setPosition(Point(0, scrollView->getInnerContainerSize().height - selectLabels.size() * size.height));
		label->setAnchorPoint(Point(0, 0.5));
		label->setPosition(Point(0, size.height / 2));
		label->setColor(Color3B(250, 160, 30));
		button->addChild(label);

		bgSprites.push_back(button);
		button->setTag((int)selectLabels.size() - 1);
		button->setSwallowTouches(true);
		button->setOpacity(250);
		scrollView->addChild(button);
	}

	//������menu item �����һ��label���ǵ�����
	void addLabel(LabelTTF* label,std::string spriteFile)
	{
		////����������м���һ����ť
		Size size = getContentSize();
		selectLabels.push_back(label);

		//float scrollInnerContainHeight = selectLabels.size() * size.height;
		//scrollView->setInnerContainerSize(Size(size.width, scrollInnerContainHeight));

		float spritePrecent = 0.3;

		ui::Button* button = ui::Button::create("listBg2.png", "listBg3.png");
		float sx = size.width / button->getContentSize().width;
		float sy = size.height / button->getContentSize().height;
		button->setScaleX(sx);
		button->setScaleY(sy);
		button->setAnchorPoint(Point(0, 0));
		button->setPosition(Point(0, scrollView->getInnerContainerSize().height - selectLabels.size() * size.height));
		label->setAnchorPoint(Point(0, 0.5));
		label->setPosition(Point(spritePrecent*button->getContentSize().width , size.height / 2));
		label->setColor(Color3B(250, 160, 30));
		button->addChild(label);

		auto sprite = Sprite::create(spriteFile);
		sprite->setPosition(spritePrecent/2 * button->getContentSize().width , size.height / 2);
		sprite->setScaleX(spritePrecent*size.width / sprite->getContentSize().width);
		sprite->setScaleY(size.height / sprite->getContentSize().height);
		button->addChild(sprite);

		bgSprites.push_back(button);
		button->setTag((int)selectLabels.size() - 1);
		button->setSwallowTouches(true);
		button->setOpacity(250);
		scrollView->addChild(button);
	}

	//ѡ�������б��
	void onSelected(Object* sender)
	{

		ui::Button* item = dynamic_cast<ui::Button*>(sender);
		if (item)
		{
			lastSelectedIndex = item->getTag();
			showLabel->setScaleX(selectLabels[item->getTag()]->getScaleX());
			showLabel->setString(selectLabels[item->getTag()]->getString());
			
		}
		onClose(); //�ر������б��
	}


	//�ر������б��
	void onClose()
	{
		//removeChild(mainMenu, true);  //ͨ��ɾ��mainMenu,�ر������б��
		isShowMenu = false;
		scrollView->setVisible(false);

		if (DropDownList::isShowOneList == true) {
			if (DropDownList::nowShowListId == this->m_listId) {
				DropDownList::isShowOneList = false;
				DropDownList::nowShowListId = 0;
			}
		}
	}


	/////Ϊ�����б������ⲿ�ص�����
	void setItemCallBack(const cocos2d::ui::Widget::ccWidgetTouchCallback& func) {
		callfunc = func;
		std::vector<ui::Button*>::iterator itor = bgSprites.begin();
		while (itor != bgSprites.end()) {
			(*itor)->addTouchEventListener(func);
			itor++;
		}
	}

	static bool isShowOneList;
	static int listId;
	static int nowShowListId;

	int m_listId;

	ui::ScrollView* scrollView;

private:
	cocos2d::ui::Widget::ccWidgetTouchCallback callfunc;

	////����ͼƬ��ѡ�е����������������б�ı���
	Sprite* selectBg;
	//std::vector<MenuItem*> bgSprites; //�������� ����
	std::vector<ui::Button*> bgSprites; //�������� ����

	//Menu* mainMenu;  //�����б�ѡ��ļ���

	LabelTTF* showLabel;  //��ʾѡ�еĽ��

	std::vector<LabelTTF*> selectLabels;  //�����б�label

	bool isShowMenu;  //�Ƿ���ʾ�������б�

	int lastSelectedIndex;  //ѡ�������б��index

							/// ������
	
	Size scrollViewSize;

	bool isTouchBegin;
};



#endif
