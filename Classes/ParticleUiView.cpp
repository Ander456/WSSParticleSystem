#include "ParticleUiView.h"
#include "FileCenter.h"

using namespace ui;
ParticleUiView::ParticleUiView() {
	refSprite = nullptr;
	mainRootNode = nullptr;
	singleRootNode = nullptr;
	isSingleModeTailPanelOpen = false;
	isSingleModeFireProLayerOpen = false;
	isSingleModeFireStartProPanelOpen = false;
	isSingleModeParLifeProPanelOpen = false;
	nowEditingSignalPar = nullptr;
	sourceBlend = nullptr;
	destBlend = nullptr;
	tailSpriteList = nullptr;
	textureList = nullptr;
	jsonFileList = nullptr;
	addFireProjsonFileList = nullptr;
	fireAreaMode = nullptr;
	fireStartProValueType = nullptr;
	editMode = EditMode::single;
	nowSelectedSingleProType = singleProType::singleProTypeNone;
	nowSelectedSingleProTypeStr = "";
	isDiyEditPolygonOrLines = false;
	nowTime = 0;
	lastTouchTime = 0;
	lastTouchDelay = 0;
	singlePar = nullptr;

	isStartDeleteDiyFireAreaPointBtn = false;
	isStartDeleteFireStartCurvePointBtn = false;
	deleteDiyFireAreaPointNeedTime = 2.5;
	deleteFireStartCurvePointNeedTime = 2.5;

	fireStartCurvePointMaxHeight = 0;
	isOpenFireStartCurvePanel = false;

	parLifeCurvePointMaxHeight = 0;
	parLifeCurvePointLeftPrecent = 0;
	parLifeCurvePointRightPrecent = 0;

	isOpenParLifeCurvePanel = false;
	isStartDeleteParLifeCurvePointBtn = false;
	deleteParLifeCurvePointNeedTime = 2.5;
	deleteParLifeCurvePointNeedTimeCount = 2.5;

	isOpenFireStartCurveColorPanel = false;
	targetFireStartCurveColorBtn = nullptr;
	isStartDeleteFireStartCurveColorBtn = false;
	deleteFireStartCurveColorNeedTime = 2.5;
	deleteFireStartCurveColorNeedTimeCount = 2.5;

	isOpenParLifeCurveColorPanel = false;
	isStartDeleteParLifeCurveColorBtn = false;
	deleteParLifeCurveColorNeedTime = 2.5;
	deleteParLifeCurveColorNeedTimeCount = 2.5;

	isOpenSetColorPanel = false;

	colorRect = nullptr;
	colorLine = nullptr;
	randColorLine = nullptr;
	fireStartCurveColorLine = nullptr;
	parLifeCurveColorLine = nullptr;

	nowEditingColorPoint = nullptr;

	isTouchInColorRect = false;
	isTouchInColorLine = false;
	isTouchInColorPanelMoveBar = false;

	nowEditingEmitterVarietyValue = nullptr;

	addEmitterHintText = nullptr;

	addFileHintText = nullptr;

}

ParticleUiView::~ParticleUiView() {

}

ParticleUiView* ParticleUiView::create() {
	auto uiView = new ParticleUiView();
	if (uiView && uiView->init()) {
		uiView->autorelease();
		return uiView;
	}
	CC_SAFE_DELETE(uiView);
	return nullptr;
}

static Widget* findChildren(Widget* node , std::string name) {
	Vector<Node*> children = node->getChildren();
	if (children.size() == 0) {
		return nullptr;
	}
	auto itor = children.begin();
	while (itor != children.end()) {
		if ((*itor)->getName() == name) {
			return (Widget*)(*itor);
		}
		Widget* node = findChildren((Widget*)(*itor), name);
		if (node) {
			return node;
		}

		itor++;
	}
	return nullptr;
}

Widget* ParticleUiView::seekByName(Widget* rootNode , std::string name) {
	CCASSERT(rootNode, "ParticleUiView , rootNode could not be null");

	if (rootNode->getName() == name) {
		return rootNode;
	}

	return findChildren(rootNode, name);

}

// ��UI�ڵ��е� TextField ����EditBox
void ParticleUiView::replaceTextFieldToEditBox(Widget* rootNode) {
	
	Vector<Node*> children = rootNode->getChildren();
	if (children.size() == 0) {
		return;
	}
	auto itor = children.begin();
	while (itor != children.end()) {
		auto widgetNode = (Widget*)(*itor);

		if (widgetNode->widgetClassName == "TextField") {
			auto widgetText = (TextField*)(*itor);
			auto editBox = ExEditBox::create(Size(widgetText->getContentSize().width * widgetText->getScaleX(), widgetText->getContentSize().height * widgetText->getScaleY()), ui::Scale9Sprite::create("editBg.png"));

			editBox->setAnchorPoint(widgetText->getAnchorPoint());
			editBox->setPosition(widgetText->getPosition());
			editBox->setInputMode(ExEditBox::InputMode::NUMERIC);
			editBox->setFontColor(Color3B::BLACK);
			editBox->setFontSize(widgetText->getFontSize());
			editBox->setName(widgetText->getName());
			editBox->setDelegate(this);
			rootNode->addChild(editBox , widgetText->getLocalZOrder() );

			(*itor)->removeFromParent();
		}
		if (widgetNode->widgetClassName == "Slider") {
			auto widgetSlider = (Slider*)(*itor);
			auto slider = ExSlider::create("sliderTrack.png", "sliderProgress.png", "sliderThumb.png");
			slider->setTouchEnabled(true);
			slider->setEnabled(false);
			slider->setSwallowsTouches(false);
			slider->setAnchorPoint(widgetSlider->getAnchorPoint());
			slider->setPosition( Vec2(widgetSlider->getPosition()));
			//slider->setContentSize(Size(widgetSlider->getContentSize().width , widgetSlider->getContentSize().height ));
			slider->setScaleX(widgetSlider->getScaleX());
			slider->setScaleY(widgetSlider->getScaleY());
			slider->setMinimumValue(0);
			slider->setMaximumValue(100);
			slider->setName(widgetSlider->getName());
			rootNode->addChild(slider, widgetSlider->getLocalZOrder());

			slider->addTargetWithActionForControlEvents(this, cccontrol_selector(ParticleUiView::onSliderChanged), extension::Control::EventType::VALUE_CHANGED);

			(*itor)->removeFromParent();
		}
		if (widgetNode->widgetClassName == "CheckBox") {
			auto checkBox = (CheckBox*)(*itor);
			checkBox->setSelectedState(false);
			checkBox->addEventListener(CC_CALLBACK_2(ParticleUiView::checkBoxEvent, this));
		}


		replaceTextFieldToEditBox((Widget*)(*itor));
		itor++;
	}
	return;
}

bool ParticleUiView::init() {
	this->scheduleUpdate();

	// test bg 
	/*auto bgSprite = Sprite::create("bg1.png");
	Size screenSize = Director::getInstance()->getOpenGLView()->getDesignResolutionSize();
	bgSprite->setAnchorPoint(Vec2(0, 0));
	bgSprite->setScaleX(screenSize.width / bgSprite->getContentSize().width);
	bgSprite->setScaleY(screenSize.height / bgSprite->getContentSize().height);
	this->addChild(bgSprite, -100);*/


	// �����¼�
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(false);
	listener->onTouchBegan = CC_CALLBACK_2(ParticleUiView::onTouchBegan, this);
	listener->onTouchMoved = CC_CALLBACK_2(ParticleUiView::onTouchMoved, this);
	listener->onTouchEnded = CC_CALLBACK_2(ParticleUiView::onTouchEnded, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);  //���¼���ӵ�this�������
	this->setTouchEnabled(true);

	// ��ȡ����ͼƬ�б�
	texNames = this->getPngFileName();

	// ��ȡ�ļ��б�
	fileNames = this->getJsonFileName();

	// ��ȡ�ο�ͼƬ�б�
	refSpriteNames = this->getRefSpriteName();

	tailSpriteNames = this->getTailSpriteName();

	// �������������
	std::srand((unsigned int)time(0));

	

	// ���˵� ���
	mainRootNode = GUIReader::getInstance()->widgetFromJsonFile("main_ui.ExportJson");
	this->addChild(mainRootNode,10);
	replaceTextFieldToEditBox(mainRootNode);

	((ExEditBox*)seekByName(mainRootNode, "fileNameInput"))->setInputMode(ExEditBox::InputMode::ANY);

	// ��һ��������panel ui
	singleRootNode = GUIReader::getInstance()->widgetFromJsonFile("single_par_ui.ExportJson");
	this->addChild(singleRootNode,10);
	replaceTextFieldToEditBox(singleRootNode);

	
	this->initUi();


	// ����������ӷ�����
	//parList = ParticleEmitterAdapter::create();
	//parList->setPosition(Point(500, 500));
	//this->addChild(parList);

	//parList->addChildEmitterFirePro();

	////��ȡidΪ1�����ӷ���������
	//nowEditingSignalPar = parList->getChildEmitter(1);
	//this->initSingleParUi();

	singlePar = pp::ParticleEmitter::create();
	//singlePar->setTestData();
	//singlePar->setRunningLayer(this);
	singlePar->setPosition(Vec2(500,500));
	this->addChild(singlePar);

	// -- �����б�ĵ�һ���ļ�
	nowFileName = "json/" + fileNames.at(0);

	singlePar->readJsonDataFromFile(nowFileName);

	//nowEditingSignalPar->readJsonData(FileCenter::getInstance()->readJsonData(nowFileName));
	

	nowEditingSignalPar = singlePar->getFirstFirePro();
	if (nowEditingSignalPar) {
		this->initSingleParUi();
	}

	this->initMoreParPanel();

	jsonFileList->setSelectedIndex(0);

	return true;
}

// ��ʾ���firePro�����
void ParticleUiView::showAddFireProPanel() {
	isOpenAddFireProPanel = true;
	seekByName(mainRootNode, "addFireProPanel")->setVisible(true);
	((CheckBox*)seekByName(mainRootNode, "addRenderFireProCheckBox"))->setSelectedState(true);
	((CheckBox*)seekByName(mainRootNode, "addEmitterFireProCheckBox"))->setSelectedState(false);

	addFireProJsonfileNames.clear();

	std::vector<std::string>::iterator itor = fileNames.begin();
	while (itor != fileNames.end()) {
		if ("json/" + (*itor) != nowFileName) {
			addFireProJsonfileNames.push_back((*itor));
		}
		itor++;
	}

	if (addFireProJsonfileNames.size() > 0) {
		// ����һ�������Լ�����ļ�������json�ļ��������б�
		if (!addFireProjsonFileList) {
			{
				//----- ���� ������ json�ļ� ������
				LabelTTF* initLabel = LabelTTF::create("jsonFile", "Arial", 22);
				//������ʾ��Ŀ��size  
				auto textureSprite = seekByName(mainRootNode, "emitterFileSprite");
				Size size = CCSizeMake(textureSprite->getContentSize().width * textureSprite->getScaleX(), textureSprite->getContentSize().height  * textureSprite->getScaleY());
				addFireProjsonFileList = DropDownList::create(initLabel, size, addFireProJsonfileNames.size());
				addFireProjsonFileList->setPosition(textureSprite->getPositionX() - size.width / 2, textureSprite->getPositionY() - size.height / 2);
				addFireProjsonFileList->setSwallowsTouches(true);
				textureSprite->getParent()->addChild(addFireProjsonFileList, textureSprite->getLocalZOrder());

				////���ݶ���������png�ļ������֣���������Ӧ��label
				std::vector<std::string>::iterator texItor = addFireProJsonfileNames.begin();
				while (texItor != addFireProJsonfileNames.end()) {
					auto label = LabelTTF::create((*texItor), "Arial", 22);
					addFireProjsonFileList->addLabel(label);
					texItor++;
				}
				addFireProjsonFileList->setName("addFireProjsonFileList");
				addFireProjsonFileList->setItemCallBack(CC_CALLBACK_2(ParticleUiView::onDropDownList, this, addFireProjsonFileList));
			}

			nowAddFireProJsonFileName = addFireProJsonfileNames.at(0);
			addFireProjsonFileList->setSelectedIndex(0);
		}
	}
	else {
		nowAddFireProJsonFileName = "";
	}

}
// ���� ���firePro�����
void ParticleUiView::hideAddFireProPanel() {
	isOpenAddFireProPanel = false;
	seekByName(mainRootNode, "addFireProPanel")->setVisible(false);
}

void ParticleUiView::showAddFilePanel() {
	isOpenAddFilePanel = true;
	seekByName(mainRootNode, "addFilePanel")->setVisible(true);
}

void ParticleUiView::hideAddFilePanel() {
	isOpenAddFilePanel = false;
	seekByName(mainRootNode, "addFilePanel")->setVisible(false);
}


Widget* ParticleUiView::addOneToMoreParPanel(std::string showWord , std::string tagName , int id , int localZorder) {
	auto scrollView = (ScrollView*)seekByName(mainRootNode, "moreParFileScrollView");
	Size scrollCSize = scrollView->getContentSize();
	scrollView->setTouchEnabled(true);
	scrollView->setBounceEnabled(true);

	auto item = ImageView::create("fileListItemBg2.png");
	Size cSize = item->getContentSize();

	item->setTag(id);

	Size innerSize = Size(scrollCSize.width, (moreParWidgetVec.size() + 1) * cSize.height);
	if (innerSize.height < scrollCSize.height) {
		innerSize.height = scrollCSize.height;
	}
	else {
		for (int i = 0; i < moreParWidgetVec.size(); ++i) {
			moreParWidgetVec.at(i)->setPositionY(innerSize.height - (i + 1) * cSize.height);
		}
	}
	scrollView->setInnerContainerSize(innerSize);

	item->setAnchorPoint(Vec2(0, 0));
	item->setPosition(Vec2(0, innerSize.height - (moreParWidgetVec.size() + 1) * cSize.height));

	scrollView->addChild(item);
	scrollView->scrollToBottom(0.5, true);

	item->setTouchEnabled(true);
	item->addTouchEventListener([this,item](Ref* pSender, Widget::TouchEventType eType) {
		if (eType == Widget::TouchEventType::ENDED) {
			this->nowEditingSignalPar = this->getSignalPar()->getFireProById(item->getTag());

			auto itemItor = moreParWidgetVec.begin();
			while (itemItor != moreParWidgetVec.end()) {
				if ((*itemItor) == item) {
					(*itemItor)->setColor(Color3B(255, 255, 0));
				}
				else{
					(*itemItor)->setColor(Color3B(255, 255, 255));
				}

				itemItor++;
			}

			this->initSingleParUi();
			// �ص����е����
			//this->setTailProPanelVisible(false);
			//this->setFireProLayerVisible(false);
			this->hideFireStartProValuePanel();
			this->hideParLifeProValuePanel();
		}
	});

	// �ı�
	auto textEditBox = ui::Text::create();
	textEditBox->setAnchorPoint(Vec2(0.5, 0.5));
	textEditBox->setPosition(Vec2(0.35 * cSize.width , 0.65*cSize.height));
	textEditBox->setFontSize(20);
	item->addChild(textEditBox);
	textEditBox->setText(showWord);

	// id �ı�
	auto idText = ui::Text::create();
	idText->setAnchorPoint(Vec2(0.5, 0.5));
	idText->setPosition(Vec2(0.1 * cSize.width, 0.5*cSize.height));
	idText->setFontSize(20);
	idText->setName("idText");
	item->addChild(idText);
	char idStr[20];
	sprintf(idStr , "%d" , id);
	idText->setText(idStr);

	// localZorder Editbox
	{
		auto textEditBox = ExEditBox::create(Size(35, 25), ui::Scale9Sprite::create("editBg.png"));
		textEditBox->setAnchorPoint(Vec2(0.5, 0.5));
		textEditBox->setPosition(Vec2(0.7 * cSize.width, 0.5*cSize.height));
		textEditBox->setInputMode(ExEditBox::InputMode::ANY);
		textEditBox->setFontColor(Color3B::BLACK);
		textEditBox->setFontSize(20);
		char name[20];
		textEditBox->setTag(9527);
		sprintf(name, "localZorder_%d", id);
		textEditBox->setName(name);
		textEditBox->setDelegate(this);
		item->addChild(textEditBox);

		
		char zorderStr[20];
		sprintf(zorderStr, "%d", localZorder);
		textEditBox->setText(zorderStr);
	}


	{
		auto textEditBox = ExEditBox::create(Size(80, 25), ui::Scale9Sprite::create("editBg.png"));
		textEditBox->setAnchorPoint(Vec2(0.5, 0.5));
		textEditBox->setPosition(Vec2(0.35 * cSize.width , 0.35*cSize.height));
		textEditBox->setInputMode(ExEditBox::InputMode::ANY);
		textEditBox->setFontColor(Color3B::BLACK);
		textEditBox->setFontSize(20);
		char name[20];
		textEditBox->setTag(9527);
		sprintf(name, "tagName_%d", id );
		textEditBox->setName(name);
		textEditBox->setDelegate(this);
		item->addChild(textEditBox);

		textEditBox->setText(tagName.c_str());
	}
	// ��ť
	auto subBtn = ImageView::create("sub.png");
	subBtn->setTouchEnabled(true);
	subBtn->setPosition(Vec2(0.9 * cSize.width , cSize.height / 2));
	subBtn->setScale(0.5);
	item->addChild(subBtn);

	subBtn->addTouchEventListener([this, item, scrollView](Ref* pSender, Widget::TouchEventType eType) {
		if (eType == Widget::TouchEventType::ENDED) {
			auto itor = this->moreParWidgetVec.begin();
			int index = 0;
			while (itor != this->moreParWidgetVec.end()) {
				if ((*itor) == item) {

					Size innerSize = Size(item->getContentSize().width, (this->moreParWidgetVec.size() - 1) * item->getContentSize().height);
					if (innerSize.height < scrollView->getContentSize().height) {
						innerSize.height = scrollView->getContentSize().height;
					}
					scrollView->setInnerContainerSize(innerSize);


					if (this->nowEditingSignalPar->_id == (*itor)->getTag()) {
						//// ɾ�����Զ�ѡ��һ��firePro
						//�������ң���û�У���������
						bool isGetNewFirePro = false;
						Widget* newItemWidget = nullptr;
						if (index - 1 >= 0) {
							newItemWidget = this->moreParWidgetVec.at(index - 1);
							this->nowEditingSignalPar = this->getSignalPar()->getFireProById(newItemWidget->getTag());
							isGetNewFirePro = true;
						}
						else if (index+1 < this->moreParWidgetVec.size()) {
							newItemWidget = this->moreParWidgetVec.at(index+1);
							this->nowEditingSignalPar = this->getSignalPar()->getFireProById(newItemWidget->getTag());
							isGetNewFirePro = true;
						}
						else {
							this->nowEditingSignalPar = nullptr;
						}
						if (isGetNewFirePro) {
							auto itemItor = moreParWidgetVec.begin();
							while (itemItor != moreParWidgetVec.end()) {
								if ((*itemItor) == newItemWidget) {
									(*itemItor)->setColor(Color3B(255, 255, 0));
								}
								else {
									(*itemItor)->setColor(Color3B(255, 255, 255));
								}

								itemItor++;
							}

							this->initSingleParUi();
							// �ص����е����
							this->hideFireStartProValuePanel();
							this->hideParLifeProValuePanel();
						}
					}
					
					// ɾ��
					this->getSignalPar()->removeFireProById(item->getTag());
					float cHeight = item->getContentSize().height;
					(*itor)->removeFromParent();
					this->moreParWidgetVec.erase(itor);

					for (int i = 0; i < this->moreParWidgetVec.size(); ++i) {
						this->moreParWidgetVec.at(i)->setPositionY(innerSize.height - (i + 1) * cHeight);
						// �޸�id
						int oldid = this->moreParWidgetVec.at(i)->getTag();
						this->getSignalPar()->getFireProById(oldid)->_id = i + 1;
						this->moreParWidgetVec.at(i)->setTag(i + 1);
						char idStr[20];
						sprintf(idStr, "%d", i + 1);
						((ui::Text*)this->moreParWidgetVec.at(i)->getChildByName("idText"))->setText(idStr);

						this->getSignalPar()->setChildrenParNewId(oldid, i + 1);
					}


					ParticleEmitter::fireProId = this->moreParWidgetVec.size()+1;


					break;
				}
				itor++;
				index++;
			}

		}
	});

	moreParWidgetVec.push_back(item);

	return item;
}

void ParticleUiView::initMoreParPanel() {
	if (singlePar) {

		auto itor = singlePar->fireProVec.begin();
		while (itor != singlePar->fireProVec.end()) {
			Widget* item = nullptr;
			if ((*itor)->_parType == parType::render) {
				item = this->addOneToMoreParPanel("render" , (*itor)->_tagName, (*itor)->_id , (*itor)->_localZorder);
			}
			else if ((*itor)->_parType == parType::emitter) {
				item = this->addOneToMoreParPanel((*itor)->_emitterFileName, (*itor)->_tagName, (*itor)->_id, (*itor)->_localZorder);
			}
			if (item) {
				if (item->getTag() == nowEditingSignalPar->_id) {
					item->setColor(Color3B(255, 255, 0));
				}
			}

			itor++;
		}
	}
}

void ParticleUiView::clearMoreParPanel() {
	auto itor = moreParWidgetVec.begin();
	while (itor != moreParWidgetVec.end()) {
		(*itor)->removeFromParent();
		itor++;
	}
	moreParWidgetVec.clear();
}

void ParticleUiView::initUi() {
	// ---------------------------  main panel
	

	/*auto moreParMode = ((CheckBox*)seekByName(mainRootNode, "moreParMode"));
	moreParMode->setSelectedState(false);
	moreParMode->addEventListener(CC_CALLBACK_2(ParticleUiView::checkBoxEvent, this));

	auto DrawNodeCheckBox = ((CheckBox*)seekByName(mainRootNode, "isUseDrawNodeCheckBox"));
	DrawNodeCheckBox->setSelectedState(false);
	DrawNodeCheckBox->addEventListener(CC_CALLBACK_2(ParticleUiView::checkBoxEvent, this));*/

	seekByName(mainRootNode, "singleParModeFilePanel")->setVisible(true);
	seekByName(mainRootNode, "moreParModeProPanel")->setVisible(true);

	seekByName(mainRootNode, "saveSingleParFileBtn")->addTouchEventListener([this](Ref* pSender, Widget::TouchEventType eType) {
		if (eType == Widget::TouchEventType::ENDED) {
			FileCenter::getInstance()->writeJsonData(nowFileName, this->getSignalPar());
		}
	});

	{
		//----- ���� ������ json�ļ� ������
		LabelTTF* initLabel = LabelTTF::create("jsonFile", "Arial", 22); 
		//������ʾ��Ŀ��size  
		auto textureSprite = seekByName(mainRootNode, "singleParFileSprite");
		Size size = CCSizeMake(textureSprite->getContentSize().width * textureSprite->getScaleX(), textureSprite->getContentSize().height  * textureSprite->getScaleY());
		jsonFileList = DropDownList::create(initLabel, size, fileNames.size());
		jsonFileList->setPosition(textureSprite->getPositionX() - size.width / 2, textureSprite->getPositionY() - size.height / 2);
		jsonFileList->setSwallowsTouches(true);
		textureSprite->getParent()->addChild(jsonFileList, textureSprite->getLocalZOrder());

		////���ݶ���������png�ļ������֣���������Ӧ��label
		std::vector<std::string>::iterator texItor = fileNames.begin();
		while (texItor != fileNames.end()) {
			auto label = LabelTTF::create((*texItor), "Arial", 22);
			jsonFileList->addLabel(label);
			texItor++;
		}
		jsonFileList->setName("jsonFileList");
		jsonFileList->setItemCallBack(CC_CALLBACK_2(ParticleUiView::onDropDownList, this, jsonFileList));
	}

	{
		//----- �ο�ͼƬ��������
		LabelTTF* initLabel = LabelTTF::create("refSprite", "Arial", 22);
		//������ʾ��Ŀ��size  
		auto textureSprite = seekByName(mainRootNode, "bgSprite");
		Size size = CCSizeMake(textureSprite->getContentSize().width * textureSprite->getScaleX(), textureSprite->getContentSize().height  * textureSprite->getScaleY());
		refSpriteList = DropDownList::create(initLabel, size, refSpriteNames.size() + 1);
		refSpriteList->setPosition(textureSprite->getPositionX() - size.width / 2, textureSprite->getPositionY() - size.height / 2);
		refSpriteList->setSwallowsTouches(true);
		textureSprite->getParent()->addChild(refSpriteList, textureSprite->getLocalZOrder());

		auto label = LabelTTF::create("Black", "Arial", 22);
		refSpriteList->addLabel(label);

		////���ݶ���������png�ļ������֣���������Ӧ��label
		std::vector<std::string>::iterator texItor = refSpriteNames.begin();
		while (texItor != refSpriteNames.end()) {
			auto label = LabelTTF::create((*texItor), "Arial", 22);
			refSpriteList->addLabel(label , "refSprite/" + (*texItor));
			texItor++;
		}
		refSpriteList->setName("refSpriteList");
		refSpriteList->setItemCallBack(CC_CALLBACK_2(ParticleUiView::onDropDownList, this, refSpriteList));
	}

	seekByName(mainRootNode, "addParFileToScrollView")->addTouchEventListener([this](Ref* pSender, Widget::TouchEventType eType) {
		if (eType == Widget::TouchEventType::ENDED) {
			this->showAddFireProPanel();
		}
	});

	seekByName(mainRootNode, "addSingleParFileBtn")->addTouchEventListener([this](Ref* pSender, Widget::TouchEventType eType) {
		if (eType == Widget::TouchEventType::ENDED) {
			this->showAddFilePanel();
		}
	});

	seekByName(mainRootNode, "addFileNameBtn")->addTouchEventListener([this](Ref* pSender, Widget::TouchEventType eType) {
		if (eType == Widget::TouchEventType::ENDED) {
			std::string addFile = ((EditBox*)seekByName(mainRootNode, "fileNameInput"))->getText();
			
			addFile += ".par";

			bool isHaveSameFile = false;
			for (int i = 0; i < fileNames.size(); ++i) {
				if (addFile == fileNames.at(i)) {
					isHaveSameFile = true;
					break;
				}
			}
			if (!isHaveSameFile) {
				fileNames.push_back(addFile);

				auto label = LabelTTF::create( addFile, "Arial", 22);
				jsonFileList->addLabelDynamic(label);

				jsonFileList->setSelectedIndexByString(addFile);

				jsonFileList->scrollView->scrollToBottom(1, false);

				nowFileName = "json/" + addFile;

				FileCenter::instance->writeJsonData(nowFileName, nullptr);

				singlePar->stopSystem();
				singlePar->clearData();
				this->clearMoreParPanel();
				singlePar->readJsonDataFromFile(nowFileName);
				nowEditingSignalPar = singlePar->getFirstFirePro();
				this->initMoreParPanel();
				singlePar->resetSystem();
				// �ص����е����
				this->setTailProPanelVisible(false);
				this->setFireProLayerVisible(false);
				isSingleModeFireProLayerOpen = false;
				this->hideFireStartProValuePanel();
				this->hideParLifeProValuePanel();


			}
			else {
				if (!addFileHintText) {
					addFileHintText = ui::Text::create();
					addFileHintText->setPosition(Vec2(Director::getInstance()->getVisibleSize().width / 2, Director::getInstance()->getVisibleSize().height / 2));
					addFileHintText->setFontSize(40);
					addFileHintText->setColor(Color3B(255, 0, 0));
					this->addChild(addFileHintText,100);
					addFileHintText->setText("the file U add is exist!");
				}

				addFileHintText->setOpacity(255);
				addFileHintText->runAction(CCFadeOut::create(6));
			}
		}
	});

	seekByName(mainRootNode, "addFireProCtnBtn")->addTouchEventListener([this](Ref* pSender, Widget::TouchEventType eType) {
		if (eType == Widget::TouchEventType::ENDED) {
			emitterFirePro* newFirePro = nullptr;
			if (((CheckBox*)seekByName(mainRootNode, "addRenderFireProCheckBox"))->getSelectedState() == true) {
				newFirePro = this->getSignalPar()->addOneFirePro(-1, parType::render, "");
				this->getSignalPar()->setTestData(*newFirePro);
				this->getSignalPar()->addRender();
			}
			else if (((CheckBox*)seekByName(mainRootNode, "addEmitterFireProCheckBox"))->getSelectedState() == true) {
				if (nowAddFireProJsonFileName != "") {
					// �ж� ���Ҫ��ӵ��ļ������Ƿ��� emitter �ķ������͡�������������
					rapidjson::Document& readDoc = FileCenter::instance->readJsonData("json/" + nowAddFireProJsonFileName);
					
					bool isEmitter = false;
					if (readDoc.IsArray()) {
						rapidjson::Value& array = readDoc;
						for (int i = 0; i < array.Size(); i++) {
							std::string str = array[i]["parType"].GetString();
							if (str == "emitter") {
								isEmitter = true;
								break;
							}
						}
					}
					if (!isEmitter) {
						newFirePro = this->getSignalPar()->addOneFirePro(-1, parType::emitter, nowAddFireProJsonFileName);
						this->getSignalPar()->setTestData(*newFirePro);
					}
					else {
						if (!addEmitterHintText) {
							addEmitterHintText = ui::Text::create();
							addEmitterHintText->setPosition(Vec2(Director::getInstance()->getVisibleSize().width / 2, Director::getInstance()->getVisibleSize().height / 2));
							addEmitterHintText->setFontSize(40);
							addEmitterHintText->setColor(Color3B(0,255,255));
							this->addChild(addEmitterHintText);
							addEmitterHintText->setText("the file U add have other emitter file , add emitter file just can nested loop 2 !");
						}

						addEmitterHintText->setOpacity(255);
						addEmitterHintText->runAction(CCFadeOut::create(6));

					}

				}
			}
			/*if (!this->getNowEditingSignalPar() && newFirePro) {
				this->setNowEditingSignalPar(newFirePro);
				this->initSingleParUi();
			}*/

			/// ��ǰ�༭�ķ���������������
			this->setNowEditingSignalPar(newFirePro);
			this->initSingleParUi();


			this->hideAddFireProPanel();
			//
			this->clearMoreParPanel();
			this->initMoreParPanel();

		}
	});

	seekByName(mainRootNode, "cancelFireProCtnBtn")->addTouchEventListener([this](Ref* pSender, Widget::TouchEventType eType) {
		if (eType == Widget::TouchEventType::ENDED) {
			this->hideAddFireProPanel();
		}
	});


	// ---------------------------  single par panel
	

	((ScrollView*)seekByName(singleRootNode, "top_pro_layer"))->setTouchEnabled(false);
	((ScrollView*)seekByName(singleRootNode, "top_pro_layer"))->setBounceEnabled(false);
	seekByName(singleRootNode, "fireProLayer")->setVisible(false);
	this->setFireProLayerVisible(false);
	seekByName(singleRootNode, "fireStartProPanel")->setVisible(false);
	seekByName(singleRootNode, "fireStartProPanel")->setTouchEnabled(false);

	seekByName(singleRootNode, "parLifeProPanel")->setVisible(false);
	seekByName(singleRootNode, "parLifeProPanel")->setTouchEnabled(false);
	//seekByName(singleRootNode, "fireProLayer")->setSwallowTouches(true);

	// ��� ��β���� ��ť
	seekByName(singleRootNode, "tailProBtn")->addTouchEventListener([this](Ref* pSender, Widget::TouchEventType eType) {
		if (eType == Widget::TouchEventType::ENDED) {
			auto layer = seekByName(singleRootNode, "tailProPanel");
			if (isSingleModeTailPanelOpen) {
				this->setTailProPanelVisible(false);

				isSingleModeTailPanelOpen = false;
				seekByName(singleRootNode, "tailProClose")->setVisible(true);
				seekByName(singleRootNode, "tailProOpen")->setVisible(false);

				auto mapItor = fireStartProTypeBtns.find(nowSelectedSingleProTypeStr);
				if (mapItor != fireStartProTypeBtns.end()) {
					seekByName(singleRootNode, (*mapItor).first)->setColor(Color3B(255, 255, 255));
				}
				auto mapItor2 = parLifeProTypeBtns.find(nowSelectedSingleProTypeStr);
				if (mapItor2 != parLifeProTypeBtns.end()) {
					seekByName(singleRootNode, (*mapItor2).first)->setColor(Color3B(255, 255, 255));
				}

				this->hideFireStartProValuePanel();
				this->hideParLifeProValuePanel();

				auto parLifeProBtn = seekByName(singleRootNode, "parLifeProBtn");
				parLifeProBtn->setPosition(Vec2(0, seekByName(singleRootNode, "fireStartProBtn")->getPositionY() - parLifeProBtn->getContentSize().height));
			}
			else {
				this->setTailProPanelVisible(true);
				isSingleModeTailPanelOpen = true;

				this->setFireProLayerVisible(false);
				isSingleModeFireProLayerOpen = false;

				this->hideFireStartProValuePanel();
				this->hideParLifeProValuePanel();
				isSingleModeFireStartProPanelOpen = false;

				isSingleModeParLifeProPanelOpen = false;

				auto mapItor = fireStartProTypeBtns.find(nowSelectedSingleProTypeStr);
				if (mapItor != fireStartProTypeBtns.end()) {
					seekByName(singleRootNode, (*mapItor).first)->setColor(Color3B(255, 255, 255));
				}
				auto mapItor2 = parLifeProTypeBtns.find(nowSelectedSingleProTypeStr);
				if (mapItor2 != parLifeProTypeBtns.end()) {
					seekByName(singleRootNode, (*mapItor2).first)->setColor(Color3B(255, 255, 255));
				}

				auto fireStartProPanel = seekByName(singleRootNode, "fireStartProPanel");
				fireStartProPanel->setVisible(false);
				fireStartProPanel->setTouchEnabled(false);

				auto parLifeProPanel = seekByName(singleRootNode, "parLifeProPanel");
				parLifeProPanel->setVisible(false);
				parLifeProPanel->setTouchEnabled(false);

				auto parLifeProBtn = seekByName(singleRootNode, "parLifeProBtn");
				parLifeProBtn->setPosition(Vec2(0, seekByName(singleRootNode, "fireStartProBtn")->getPositionY() - parLifeProBtn->getContentSize().height));
			}
		}
	});

	// ��� ���������� ��ť
	seekByName(singleRootNode , "emitterProBtn")->addTouchEventListener([this](Ref* pSender, Widget::TouchEventType eType) {
		if (eType == Widget::TouchEventType::ENDED) {
			auto layer = seekByName(singleRootNode, "fireProLayer");
			if (isSingleModeFireProLayerOpen) {
				//layer->setVisible(false);
				//layer->setTouchEnabled(false);
				this->setFireProLayerVisible(false);
				isSingleModeFireProLayerOpen = false;
				seekByName(singleRootNode, "emitterProClose")->setVisible(true);
				seekByName(singleRootNode, "emitterProOpen")->setVisible(false);

			}
			else {
				//layer->setVisible(true);
				//layer->setTouchEnabled(true);
				this->setFireProLayerVisible(true);
				isSingleModeFireProLayerOpen = true;
				seekByName(singleRootNode, "emitterProClose")->setVisible(false);
				seekByName(singleRootNode, "emitterProOpen")->setVisible(true);

				auto fireStartProPanel = seekByName(singleRootNode, "fireStartProPanel");
				fireStartProPanel->setVisible(false);
				fireStartProPanel->setTouchEnabled(false);
				isSingleModeFireStartProPanelOpen = false;
				seekByName(singleRootNode, "fireStartProClose")->setVisible(true);
				seekByName(singleRootNode, "fireStartProOpen")->setVisible(false);

				this->setTailProPanelVisible(false);
				isSingleModeTailPanelOpen = false;

				auto mapItor = fireStartProTypeBtns.find(nowSelectedSingleProTypeStr);
				if (mapItor != fireStartProTypeBtns.end()) {
					seekByName(singleRootNode, (*mapItor).first)->setColor(Color3B(255, 255, 255));
				}

				auto parLifeProPanel = seekByName(singleRootNode, "parLifeProPanel");
				parLifeProPanel->setVisible(false);
				parLifeProPanel->setTouchEnabled(false);
				isSingleModeParLifeProPanelOpen = false;
				seekByName(singleRootNode, "parLifeProClose")->setVisible(true);
				seekByName(singleRootNode, "parLifeProOpen")->setVisible(false);

				// ��һ��ѡ�еİ�ť��������һ��ѡ��ʱ������ɫ��ԭ
				{
					auto mapItor = parLifeProTypeBtns.find(nowSelectedSingleProTypeStr);
					if (mapItor != parLifeProTypeBtns.end()) {
						seekByName(singleRootNode, (*mapItor).first)->setColor(Color3B(255, 255, 255));
					}
				}

				this->hideFireStartProValuePanel();
				this->hideParLifeProValuePanel();
				

				auto parLifeProBtn = seekByName(singleRootNode, "parLifeProBtn");
				parLifeProBtn->setPosition(Vec2(0, seekByName(singleRootNode, "fireStartProBtn")->getPositionY() - parLifeProBtn->getContentSize().height));
			}
		}
	});

	// ��� ����ʱ�������� ��ť
	seekByName(singleRootNode, "fireStartProBtn")->addTouchEventListener([this](Ref* pSender, Widget::TouchEventType eType) {
		if (eType == Widget::TouchEventType::ENDED) {
			auto layer = seekByName(singleRootNode, "fireStartProPanel");
			if (isSingleModeFireStartProPanelOpen) {
				layer->setVisible(false);
				layer->setTouchEnabled(false);
				isSingleModeFireStartProPanelOpen = false;
				seekByName(singleRootNode, "fireStartProClose")->setVisible(true);
				seekByName(singleRootNode, "fireStartProOpen")->setVisible(false); 

				auto mapItor = fireStartProTypeBtns.find(nowSelectedSingleProTypeStr);
				if (mapItor != fireStartProTypeBtns.end()) {
					seekByName(singleRootNode, (*mapItor).first)->setColor(Color3B(255, 255, 255));
				}

				this->hideFireStartProValuePanel();

				auto parLifeProBtn = seekByName(singleRootNode, "parLifeProBtn");
				parLifeProBtn->setPosition(Vec2(0, seekByName(singleRootNode, "fireStartProBtn")->getPositionY() - parLifeProBtn->getContentSize().height));
			}
			else {
				//// �ص� �Զ���༭��������
				/*isDiyEditPolygonOrLines = true;
				this->showDiyFireAreaPoint();*/

				layer->setVisible(true);
				layer->setTouchEnabled(true);
				isSingleModeFireStartProPanelOpen = true;
				seekByName(singleRootNode, "fireStartProClose")->setVisible(false);
				seekByName(singleRootNode, "fireStartProOpen")->setVisible(true);

				auto parLifeProPanel = seekByName(singleRootNode, "parLifeProPanel");
				parLifeProPanel->setVisible(false);
				parLifeProPanel->setTouchEnabled(false);
				isSingleModeParLifeProPanelOpen = false;
				seekByName(singleRootNode, "parLifeProClose")->setVisible(true);
				seekByName(singleRootNode, "parLifeProOpen")->setVisible(false);

				this->setTailProPanelVisible(false);
				isSingleModeTailPanelOpen = false;

				// ��һ��ѡ�еİ�ť��������һ��ѡ��ʱ������ɫ��ԭ
				auto mapItor = parLifeProTypeBtns.find(nowSelectedSingleProTypeStr);
				if (mapItor != parLifeProTypeBtns.end()) {
					seekByName(singleRootNode, (*mapItor).first)->setColor(Color3B(255, 255, 255));
				}
				this->hideParLifeProValuePanel();

				auto fireProLayer = seekByName(singleRootNode, "fireProLayer");
				//fireProLayer->setVisible(false);
				//fireProLayer->setTouchEnabled(false);
				this->setFireProLayerVisible(false);
				isSingleModeFireProLayerOpen = false;
				seekByName(singleRootNode, "emitterProClose")->setVisible(true);
				seekByName(singleRootNode, "emitterProOpen")->setVisible(false);

				//
				auto parLifeProBtn = seekByName(singleRootNode, "parLifeProBtn");
				parLifeProBtn->setPosition(Vec2(0 , seekByName(singleRootNode, "fireStartProBtn")->getPositionY() - seekByName(singleRootNode, "fireStartProPanel")->getContentSize().height - parLifeProBtn->getContentSize().height));

			}
		}
	});

	// ��� �������������� ��ť
	seekByName(singleRootNode, "parLifeProBtn")->addTouchEventListener([this](Ref* pSender, Widget::TouchEventType eType) {
		if (eType == Widget::TouchEventType::ENDED) {
			auto layer = seekByName(singleRootNode, "parLifeProPanel");
			if (isSingleModeParLifeProPanelOpen) {
				layer->setVisible(false);
				layer->setTouchEnabled(false);
				isSingleModeParLifeProPanelOpen = false;
				seekByName(singleRootNode, "parLifeProClose")->setVisible(true);
				seekByName(singleRootNode, "parLifeProOpen")->setVisible(false);

				// ��һ��ѡ�еİ�ť��������һ��ѡ��ʱ������ɫ��ԭ
				auto mapItor = parLifeProTypeBtns.find(nowSelectedSingleProTypeStr);
				if (mapItor != parLifeProTypeBtns.end()) {
					seekByName(singleRootNode, (*mapItor).first)->setColor(Color3B(255, 255, 255));
				}
				this->hideParLifeProValuePanel();
			}
			else {
				layer->setVisible(true);
				layer->setTouchEnabled(true);
				isSingleModeParLifeProPanelOpen = true;
				seekByName(singleRootNode, "parLifeProClose")->setVisible(false);
				seekByName(singleRootNode, "parLifeProOpen")->setVisible(true);

				auto fireStartProPanel = seekByName(singleRootNode, "fireStartProPanel");
				fireStartProPanel->setVisible(false);
				fireStartProPanel->setTouchEnabled(false);
				isSingleModeFireStartProPanelOpen = false;
				seekByName(singleRootNode, "fireStartProClose")->setVisible(true);
				seekByName(singleRootNode, "fireStartProOpen")->setVisible(false);

				this->setTailProPanelVisible(false);
				isSingleModeTailPanelOpen = false;

				auto mapItor = fireStartProTypeBtns.find(nowSelectedSingleProTypeStr);
				if (mapItor != fireStartProTypeBtns.end()) {
					seekByName(singleRootNode, (*mapItor).first)->setColor(Color3B(255, 255, 255));
				}

				auto fireProLayer = seekByName(singleRootNode, "fireProLayer");
				//fireProLayer->setVisible(false);
				//fireProLayer->setTouchEnabled(false);
				this->setFireProLayerVisible(false);
				isSingleModeFireProLayerOpen = false;
				seekByName(singleRootNode, "emitterProClose")->setVisible(true);
				seekByName(singleRootNode, "emitterProOpen")->setVisible(false);

				this->hideFireStartProValuePanel();

				auto parLifeProBtn = seekByName(singleRootNode, "parLifeProBtn");
				parLifeProBtn->setPosition(Vec2(0, seekByName(singleRootNode, "fireStartProBtn")->getPositionY() - parLifeProBtn->getContentSize().height));
			}
		}
	});
	

	//
	/*auto isLoop = ((CheckBox*)seekByName(singleRootNode, "isLoopCheckBox"));
	isLoop->setSelectedState(true);
	isLoop->addEventListener(CC_CALLBACK_2(ParticleUiView::checkBoxEvent, this));

	auto moveModeFree = ((CheckBox*)seekByName(singleRootNode, "moveMode_free"));
	moveModeFree->setSelectedState(false);
	moveModeFree->addEventListener(CC_CALLBACK_2(ParticleUiView::checkBoxEvent, this));

	auto moveModeRelative = ((CheckBox*)seekByName(singleRootNode, "moveMode_relative"));
	moveModeRelative->setSelectedState(true);
	moveModeRelative->addEventListener(CC_CALLBACK_2(ParticleUiView::checkBoxEvent, this));

	auto angleModeGlobal = ((CheckBox*)seekByName(singleRootNode, "angleMode_global"));
	angleModeGlobal->addEventListener(CC_CALLBACK_2(ParticleUiView::checkBoxEvent, this));
	angleModeGlobal->setSelectedState(true);
	
	auto angleModeLocal = ((CheckBox*)seekByName(singleRootNode, "angleMode_local"));
	angleModeLocal->addEventListener(CC_CALLBACK_2(ParticleUiView::checkBoxEvent, this));
	angleModeLocal->setSelectedState(false);*/
	

	{
		//----- ��������ͼƬ������
		LabelTTF* initLabel = LabelTTF::create("texture", "Arial", 22);
		//������ʾ��Ŀ��size  
		auto textureSprite = seekByName(singleRootNode, "textureSprite");
		Size size = CCSizeMake(textureSprite->getContentSize().width * textureSprite->getScaleX(), textureSprite->getContentSize().height  * textureSprite->getScaleY());
		textureList = DropDownList::create(initLabel, size, texNames.size());
		textureList->setPosition(textureSprite->getPositionX() - size.width/2 , textureSprite->getPositionY() - size.height / 2);
		textureSprite->getParent()->addChild(textureList , textureSprite->getLocalZOrder());

		////���ݶ���������png�ļ������֣���������Ӧ��label
		std::vector<std::string>::iterator texItor = texNames.begin();
		while (texItor != texNames.end()) {
			auto label = LabelTTF::create((*texItor), "Arial", 22);
			textureList->addLabel(label, "texture/" + (*texItor));
			texItor++;
		}
		textureList->setName("textureList");
		textureList->setItemCallBack(CC_CALLBACK_2(ParticleUiView::onDropDownList, this, textureList));
	}

	// ���� ���������ģʽ
	{
		auto fireAreaModeSprite = seekByName(singleRootNode, "fireAreaModeSprite");
		Size size = CCSizeMake(fireAreaModeSprite->getContentSize().width * fireAreaModeSprite->getScaleX(), fireAreaModeSprite->getContentSize().height  * fireAreaModeSprite->getScaleY());
		LabelTTF* initLabel = LabelTTF::create("fireAreaMode", "Arial", 20);
		//initLabel->setDimensions(Size(size.width, size.height));
		//������ʾ��Ŀ��size 
		fireAreaMode = DropDownList::create(initLabel, size, 4);
		fireAreaMode->setSwallowsTouches(true);
		LabelTTF* label1 = LabelTTF::create("rect", "Arial", 22);
		fireAreaMode->addLabel(label1);
		LabelTTF* label2 = LabelTTF::create("circle", "Arial", 22);
		fireAreaMode->addLabel(label2);
		LabelTTF* label3 = LabelTTF::create("diy-polygon", "Arial", 22);
		fireAreaMode->addLabel(label3);
		LabelTTF* label4 = LabelTTF::create("diy-lines", "Arial", 22);
		fireAreaMode->addLabel(label4);

		fireAreaMode->setPosition(fireAreaModeSprite->getPositionX() - size.width / 2, fireAreaModeSprite->getPositionY() - size.height / 2);
		fireAreaModeSprite->getParent()->addChild(fireAreaMode, 0);
		fireAreaModeSprite->setVisible(false);

		fireAreaMode->setName("fireAreaMode");
		fireAreaMode->setItemCallBack(CC_CALLBACK_2(ParticleUiView::onDropDownList, this, fireAreaMode));
	}
	{
		//----- ���� Դ ���ģʽ
		auto sourceSprite = seekByName(singleRootNode, "sourceBlendSprite");
		Size size = CCSizeMake(sourceSprite->getContentSize().width * sourceSprite->getScaleX(), sourceSprite->getContentSize().height  * sourceSprite->getScaleY());
		LabelTTF* initLabel = LabelTTF::create("sourceBlend", "Arial", 20);
		//initLabel->setDimensions(Size(size.width, size.height));
		//������ʾ��Ŀ��size 
		sourceBlend = DropDownList::create(initLabel, size, 11);
		sourceBlend->setSwallowsTouches(true);
		LabelTTF* label1 = LabelTTF::create("GL_ZERO", "Arial", 22);
		sourceBlend->addLabel(label1);
		LabelTTF* label2 = LabelTTF::create("GL_ONE", "Arial", 22);
		sourceBlend->addLabel(label2);
		LabelTTF* label3 = LabelTTF::create("GL_SRC_COLOR", "Arial", 22);
		sourceBlend->addLabel(label3);
		LabelTTF* label4 = LabelTTF::create("GL_ONE_MINUS_SRC_COLOR", "Arial", 22);
		label4->setScaleX(size.width / label4->getContentSize().width);
		sourceBlend->addLabel(label4);
		LabelTTF* label5 = LabelTTF::create("GL_DST_COLOR", "Arial", 22);
		sourceBlend->addLabel(label5);
		LabelTTF* label6 = LabelTTF::create("GL_ONE_MINUS_DST_COLOR", "Arial", 22);
		label6->setScaleX(size.width / label6->getContentSize().width);
		sourceBlend->addLabel(label6);
		LabelTTF* label7 = LabelTTF::create("GL_SRC_ALPHA", "Arial", 22);
		sourceBlend->addLabel(label7);
		LabelTTF* label8 = LabelTTF::create("GL_ONE_MINUS_SRC_ALPHA", "Arial", 22);
		label8->setScaleX(size.width / label8->getContentSize().width);
		sourceBlend->addLabel(label8);
		LabelTTF* label9 = LabelTTF::create("GL_DST_ALPHA", "Arial", 22);
		sourceBlend->addLabel(label9);
		LabelTTF* label10 = LabelTTF::create("GL_ONE_MINUS_DST_ALPHA", "Arial", 22);
		label10->setScaleX(size.width / label10->getContentSize().width);
		sourceBlend->addLabel(label10);
		LabelTTF* label11 = LabelTTF::create("GL_SRC_ALPHA_SATURATE", "Arial", 22);
		label11->setScaleX(size.width / label11->getContentSize().width);
		sourceBlend->addLabel(label11);

		sourceBlend->setPosition(sourceSprite->getPositionX() - size.width / 2, sourceSprite->getPositionY() - size.height / 2);
		sourceSprite->getParent()->addChild(sourceBlend, 10);
		sourceSprite->setVisible(false);

		sourceBlend->setName("sourceBlend");
		sourceBlend->setItemCallBack(CC_CALLBACK_2(ParticleUiView::onDropDownList, this, sourceBlend));

	}

	{
		//----- ����Ŀ���ģʽ
		auto destSprite = seekByName(singleRootNode, "desBlendSprite");
		Size size = CCSizeMake(destSprite->getContentSize().width * destSprite->getScaleX(), destSprite->getContentSize().height  * destSprite->getScaleY());
		LabelTTF* initLabel = LabelTTF::create("destBlend", "Arial", 20);
		//initLabel->setDimensions(Size(size.width, size.height));
		//������ʾ��Ŀ��size 
		destBlend = DropDownList::create(initLabel, size, 11);
		destBlend->setSwallowsTouches(true);
		LabelTTF* label1 = LabelTTF::create("GL_ZERO", "Arial", 22);
		destBlend->addLabel(label1);
		LabelTTF* label2 = LabelTTF::create("GL_ONE", "Arial", 22);
		destBlend->addLabel(label2);
		LabelTTF* label3 = LabelTTF::create("GL_SRC_COLOR", "Arial", 22);
		destBlend->addLabel(label3);
		LabelTTF* label4 = LabelTTF::create("GL_ONE_MINUS_SRC_COLOR", "Arial", 22);
		label4->setScaleX(size.width / label4->getContentSize().width);
		destBlend->addLabel(label4);
		LabelTTF* label5 = LabelTTF::create("GL_DST_COLOR", "Arial", 22);
		destBlend->addLabel(label5);
		LabelTTF* label6 = LabelTTF::create("GL_ONE_MINUS_DST_COLOR", "Arial", 22);
		label6->setScaleX(size.width / label6->getContentSize().width);
		destBlend->addLabel(label6);
		LabelTTF* label7 = LabelTTF::create("GL_SRC_ALPHA", "Arial", 22);
		destBlend->addLabel(label7);
		LabelTTF* label8 = LabelTTF::create("GL_ONE_MINUS_SRC_ALPHA", "Arial", 22);
		label8->setScaleX(size.width / label8->getContentSize().width);
		destBlend->addLabel(label8);
		LabelTTF* label9 = LabelTTF::create("GL_DST_ALPHA", "Arial", 22);
		destBlend->addLabel(label9);
		LabelTTF* label10 = LabelTTF::create("GL_ONE_MINUS_DST_ALPHA", "Arial", 22);
		label10->setScaleX(size.width / label10->getContentSize().width);
		destBlend->addLabel(label10);
		LabelTTF* label11 = LabelTTF::create("GL_SRC_ALPHA_SATURATE", "Arial", 22);
		label11->setScaleX(size.width / label11->getContentSize().width);
		destBlend->addLabel(label11);

		destBlend->setPosition(destSprite->getPositionX() - size.width / 2, destSprite->getPositionY() - size.height / 2);
		destSprite->getParent()->addChild(destBlend, 0);
		destSprite->setVisible(false);

		destBlend->setName("destBlend");
		destBlend->setItemCallBack(CC_CALLBACK_2(ParticleUiView::onDropDownList, this, destBlend));
		
	}

	// ���� ����ʱ�������� �༭����ֵ���� �����б�
	{

		auto fireStartProValueTypeSprite = seekByName(singleRootNode, "typeSprite_fireStart");
		Size size = CCSizeMake(fireStartProValueTypeSprite->getContentSize().width * fireStartProValueTypeSprite->getScaleX(), fireStartProValueTypeSprite->getContentSize().height  * fireStartProValueTypeSprite->getScaleY());
		LabelTTF* initLabel = LabelTTF::create("valueType", "Arial", 20);
		//initLabel->setDimensions(Size(size.width, size.height));
		//������ʾ��Ŀ��size 
		fireStartProValueType = DropDownList::create(initLabel, size, 4);
		fireStartProValueType->setSwallowsTouches(true);
		LabelTTF* label1 = LabelTTF::create("oneConstant", "Arial", 22);
		fireStartProValueType->addLabel(label1);
		LabelTTF* label2 = LabelTTF::create("randBetweenTwoConst", "Arial", 22);
		label2->setScaleX(size.width / label2->getContentSize().width);
		fireStartProValueType->addLabel(label2);
		LabelTTF* label3 = LabelTTF::create("moreConstant", "Arial", 22);
		fireStartProValueType->addLabel(label3);
		LabelTTF* label4 = LabelTTF::create("curve", "Arial", 22);
		fireStartProValueType->addLabel(label4);

		fireStartProValueType->setPosition(fireStartProValueTypeSprite->getPositionX() - size.width / 2, fireStartProValueTypeSprite->getPositionY() - size.height / 2);
		fireStartProValueTypeSprite->getParent()->addChild(fireStartProValueType, 0);
		fireStartProValueTypeSprite->setVisible(false);
		 
		fireStartProValueType->setName("fireStartProValueType");
		fireStartProValueType->setItemCallBack(CC_CALLBACK_2(ParticleUiView::onDropDownList, this, fireStartProValueType));
	}

	// -=-=-=-=-=- ����ʱ�������Եİ�ť�Ļص�����
	
	fireStartProTypeBtns.insert(std::make_pair("fireStartBtn_life", singleProType::life_fireStart));
	fireStartProTypeBtns.insert(std::make_pair("fireStartBtn_moveSpeed", singleProType::speed_fireStart));
	fireStartProTypeBtns.insert(std::make_pair("fireStartBtn_moveAcc", singleProType::acc_fireStart));
	fireStartProTypeBtns.insert(std::make_pair("fireStartBtn_moveAngle", singleProType::moveAngle_fireStart));
	fireStartProTypeBtns.insert(std::make_pair("fireStartBtn_moveAngleSpeed", singleProType::moveAngleSpeed_fireStart));
	fireStartProTypeBtns.insert(std::make_pair("fireStartBtn_gravityX", singleProType::gravityX_fireStart));
	fireStartProTypeBtns.insert(std::make_pair("fireStartBtn_gravityY", singleProType::gravityY_fireStart));
	fireStartProTypeBtns.insert(std::make_pair("fireStartBtn_size", singleProType::size_fireStart));
	fireStartProTypeBtns.insert(std::make_pair("fireStartBtn_rotation", singleProType::rotation_fireStart));
	fireStartProTypeBtns.insert(std::make_pair("fireStartBtn_rotationSpeed", singleProType::rotationSpeed_fireStart));
	fireStartProTypeBtns.insert(std::make_pair("fireStartBtn_skewX", singleProType::skewX_fireStart));
	fireStartProTypeBtns.insert(std::make_pair("fireStartBtn_skewXSpeed", singleProType::skewXSpeed_fireStart));
	fireStartProTypeBtns.insert(std::make_pair("fireStartBtn_skewY", singleProType::skewY_fireStart));
	fireStartProTypeBtns.insert(std::make_pair("fireStartBtn_skewYSpeed", singleProType::skewYSpeed_fireStart));
	fireStartProTypeBtns.insert(std::make_pair("fireStartBtn_alpha", singleProType::alpha_fireStart));
	//fireStartProTypeBtns.insert(std::make_pair("fireStartBtn_color", singleProType::color_fireStart));

	auto itor = fireStartProTypeBtns.begin();
	while (itor != fireStartProTypeBtns.end()) {
		singleProType type = (*itor).second;
		auto typeStr = (*itor).first;
		seekByName(singleRootNode, (*itor).first )->addTouchEventListener([this, type , typeStr](Ref* pSender, Widget::TouchEventType eType) {
			if (eType == Widget::TouchEventType::ENDED) {
				if (nowSelectedSingleProType != type) {
					//nowSelectedSingleProType = type;
					// ��һ��ѡ�еİ�ť��������һ��ѡ��ʱ������ɫ��ԭ
					auto mapItor = fireStartProTypeBtns.find(nowSelectedSingleProTypeStr);
					if (mapItor != fireStartProTypeBtns.end()) {
						seekByName(singleRootNode, (*mapItor).first)->setColor(Color3B(255, 255, 255));
					}

					seekByName(singleRootNode, "fireStartBtn_color")->setColor(Color3B(255, 255, 255));
					this->hideFireStartProColorValuePanel();

					nowSelectedSingleProTypeStr = typeStr;
					((Node*)pSender)->setColor(Color3B(255,255,0));
					this->showFireStartProValuePanel(type, emitterPropertyType::oneConstant , true);
				}
				else {
					//nowSelectedSingleProType = singleProType::singleProTypeNone;
					nowSelectedSingleProTypeStr = "";
					((Node*)pSender)->setColor(Color3B(255, 255, 255));
					this->hideFireStartProValuePanel();
				}
			}
		});
		itor++;
	}
	//���俪ʼʱ �� ��ɫ ����
	seekByName(singleRootNode, "fireStartBtn_color")->addTouchEventListener([this](Ref* pSender, Widget::TouchEventType eType) {
		if (eType == Widget::TouchEventType::ENDED) {
			if (nowSelectedSingleProType != singleProType::color_fireStart) {
				//nowSelectedSingleProType = type;
				// ��һ��ѡ�еİ�ť��������һ��ѡ��ʱ������ɫ��ԭ
				auto mapItor = fireStartProTypeBtns.find(nowSelectedSingleProTypeStr);
				if (mapItor != fireStartProTypeBtns.end()) {
					seekByName(singleRootNode, (*mapItor).first)->setColor(Color3B(255, 255, 255));
				}
				nowSelectedSingleProTypeStr = "fireStartBtn_color";
				// ���ص���������
				this->hideFireStartProValuePanel();
				((Node*)pSender)->setColor(Color3B(255, 255, 0));
				this->showFireStartProColorValuePanel(singleProType::color_fireStart, emitterPropertyType::oneConstant , true);
			}
			else {
				nowSelectedSingleProTypeStr = "";
				((Node*)pSender)->setColor(Color3B(255, 255, 255));
				this->hideFireStartProValuePanel();
			}
		}
	});

	// �������������ڷ������� ��ť�ǵĵ���¼�
	{
		parLifeProTypeBtns.insert(std::make_pair("parLifeBtn_moveSpeed", singleProType::speed_parLife));
		parLifeProTypeBtns.insert(std::make_pair("parLifeBtn_moveAcc", singleProType::acc_parLife));
		parLifeProTypeBtns.insert(std::make_pair("parLifeBtn_moveAngle", singleProType::moveAngle_parLife));
		parLifeProTypeBtns.insert(std::make_pair("parLifeBtn_moveAngleSpeed", singleProType::moveAngleSpeed_parLife));
		parLifeProTypeBtns.insert(std::make_pair("parLifeBtn_gravityX", singleProType::gravityX_parLife));
		parLifeProTypeBtns.insert(std::make_pair("parLifeBtn_gravityY", singleProType::gravityY_parLife));
		parLifeProTypeBtns.insert(std::make_pair("parLifeBtn_size", singleProType::size_parLife));
		parLifeProTypeBtns.insert(std::make_pair("parLifeBtn_rotation", singleProType::rotation_parLife));
		parLifeProTypeBtns.insert(std::make_pair("parLifeBtn_rotationSpeed", singleProType::rotationSpeed_parLife));
		parLifeProTypeBtns.insert(std::make_pair("parLifeBtn_skewX", singleProType::skewX_parLife));
		parLifeProTypeBtns.insert(std::make_pair("parLifeBtn_skewXSpeed", singleProType::skewXSpeed_parLife));
		parLifeProTypeBtns.insert(std::make_pair("parLifeBtn_skewY", singleProType::skewY_parLife));
		parLifeProTypeBtns.insert(std::make_pair("parLifeBtn_skewYSpeed", singleProType::skewYSpeed_parLife));
		parLifeProTypeBtns.insert(std::make_pair("parLifeBtn_alpha", singleProType::alpha_parLife));
		//parLifeProTypeBtns.insert(std::make_pair("parLifeBtn_color", singleProType::color_parLife));

		auto itor = parLifeProTypeBtns.begin();
		while (itor != parLifeProTypeBtns.end()) {
			singleProType type = (*itor).second;
			auto typeStr = (*itor).first;
			seekByName(singleRootNode, (*itor).first)->addTouchEventListener([this, type, typeStr](Ref* pSender, Widget::TouchEventType eType) {
				if (eType == Widget::TouchEventType::ENDED) {
					if (nowSelectedSingleProType != type) {
						//nowSelectedSingleProType = type;
						// ��һ��ѡ�еİ�ť��������һ��ѡ��ʱ������ɫ��ԭ
						auto mapItor = parLifeProTypeBtns.find(nowSelectedSingleProTypeStr);
						if (mapItor != parLifeProTypeBtns.end()) {
							seekByName(singleRootNode, (*mapItor).first)->setColor(Color3B(255, 255, 255));
						}

						seekByName(singleRootNode, "parLifeBtn_color")->setColor(Color3B(255, 255, 255));
						this->hideParLifeProValuePanel();

						nowSelectedSingleProTypeStr = typeStr;
						((Node*)pSender)->setColor(Color3B(255, 255, 0)); 
						this->showParLifeProValuePanel(type);

					}
					else {
						//nowSelectedSingleProType = singleProType::singleProTypeNone;
						nowSelectedSingleProTypeStr = "";
						((Node*)pSender)->setColor(Color3B(255, 255, 255));
						this->hideParLifeProValuePanel();
					}
				}
			});
			itor++;
		}
	}
	//������������ �� ��ɫ ����
	seekByName(singleRootNode, "parLifeBtn_color")->addTouchEventListener([this](Ref* pSender, Widget::TouchEventType eType) {
		if (eType == Widget::TouchEventType::ENDED) {
			if (nowSelectedSingleProType != singleProType::color_parLife) {
				// ��һ��ѡ�еİ�ť��������һ��ѡ��ʱ������ɫ��ԭ
				auto mapItor = parLifeProTypeBtns.find(nowSelectedSingleProTypeStr);
				if (mapItor != parLifeProTypeBtns.end()) {
					seekByName(singleRootNode, (*mapItor).first)->setColor(Color3B(255, 255, 255));
				}
				nowSelectedSingleProTypeStr = "parLifeBtn_color";
				// ���ص���������
				this->hideParLifeProValuePanel();
				((Node*)pSender)->setColor(Color3B(255, 255, 0));
				this->showParLifeProValuePanel(singleProType::color_parLife);
			}
			else {
				nowSelectedSingleProTypeStr = "";
				((Node*)pSender)->setColor(Color3B(255, 255, 255));
				this->hideParLifeProValuePanel();
			}
		}
	});


	//// ��� һ�� �����ֵ ����
	seekByName(singleRootNode, "moreValue_addBtn")->addTouchEventListener([this](Ref* pSender, Widget::TouchEventType eType) {
		if (eType == Widget::TouchEventType::ENDED) {
			float value = std::atof( ((ExEditBox*)seekByName(singleRootNode, "moreValue_addInput"))->getText() );
			this->addMoreConstantWidgetVec(value);
			nowEditingEmitterVarietyValue->constValues.push_back(value);
		}
	});

	////// ��ɫ��صİ�ť
	// ��β��ɫ��ť
	seekByName(singleRootNode, "tail_color_sprite")->addTouchEventListener([this](Ref* pSender, Widget::TouchEventType eType) {
		if (eType == Widget::TouchEventType::ENDED) {
			this->showSetColorPanel();
			nowEditingColorNode = (Widget*)pSender;
			this->setPointerPosByRGB(nowEditingColorNode->getColor());
		}
	});

	// һ����ֵ��ɫ
	seekByName(singleRootNode, "oneColorValueBtn")->addTouchEventListener([this](Ref* pSender, Widget::TouchEventType eType) {
		if (eType == Widget::TouchEventType::ENDED) {
			this->showSetColorPanel();
			nowEditingColorNode = (Widget*)pSender;
			this->setPointerPosByRGB(nowEditingColorNode->getColor());
		}
	});
	// �������ֵ ��ɫ
	seekByName(singleRootNode, "randColor1")->addTouchEventListener([this](Ref* pSender, Widget::TouchEventType eType) {
		if (eType == Widget::TouchEventType::ENDED) {
			this->showSetColorPanel();
			nowEditingColorNode = (Widget*)pSender;
			this->setPointerPosByRGB(nowEditingColorNode->getColor());
		}
	});
	seekByName(singleRootNode, "randColor2")->addTouchEventListener([this](Ref* pSender, Widget::TouchEventType eType) {
		if (eType == Widget::TouchEventType::ENDED) {
			this->showSetColorPanel();
			nowEditingColorNode = (Widget*)pSender;
			this->setPointerPosByRGB(nowEditingColorNode->getColor());
		}
	});
	// �����ɫֵ
	//// ��� һ�� �����ɫֵ ����
	seekByName(singleRootNode, "moreColorValue_addBtn")->addTouchEventListener([this](Ref* pSender, Widget::TouchEventType eType) {
		if (eType == Widget::TouchEventType::ENDED) {
			Color3B addColor = Color3B(255, 255, 255);
			this->addMoreColorWidgetVec(addColor);
			nowEditingEmitterColorValue->constColors.push_back(addColor);
		}
	});


	////
	seekByName(singleRootNode, "setColorPanelCloseBtn")->addTouchEventListener([this](Ref* pSender, Widget::TouchEventType eType) {
		if (eType == Widget::TouchEventType::ENDED) {
			this->hideSetColorPanel();
		}
	});
	seekByName(singleRootNode, "setColorPanelConfirmBtn")->addTouchEventListener([this](Ref* pSender, Widget::TouchEventType eType) {
		if (eType == Widget::TouchEventType::ENDED) {
			nowEditingColorNode->setColor(Color3B(rectColor.x, rectColor.y, rectColor.z));
			if (nowSelectedSingleProType == singleProType::color_fireStart) {
				this->refreshFireStartProColorValuePanel();
			}
			else if (nowSelectedSingleProType == singleProType::color_parLife) {
				this->refreshParLifeCurveColors();
			}
			else if(isSingleModeTailPanelOpen) {
				this->getNowEditingSignalPar()->_tailPro.color = Color3B(rectColor.x, rectColor.y, rectColor.z);

				this->getNowEditingSignalPar()->_tailPro.tailNode->initWithFade(this->getNowEditingSignalPar()->_tailPro.fade, this->getNowEditingSignalPar()->_tailPro.minSeg, this->getNowEditingSignalPar()->_tailPro.stroke, this->getNowEditingSignalPar()->_tailPro.color, "tail/" + this->getNowEditingSignalPar()->_tailPro.tailName);
				
			}
		}
	});
	seekByName(singleRootNode, "setColorPanelCancelBtn")->addTouchEventListener([this](Ref* pSender, Widget::TouchEventType eType) {
		if (eType == Widget::TouchEventType::ENDED) {
			this->hideSetColorPanel();
		}
	});
	



}

void ParticleUiView::setTailProPanelVisible(bool visible) {
	seekByName(singleRootNode, "tailProPanel")->setVisible(visible);
	seekByName(singleRootNode, "tailProPanel")->setTouchEnabled(visible);

	if (visible) {
		if (!tailSpriteList) {
			//----- �ο�ͼƬ��������
			LabelTTF* initLabel = LabelTTF::create("tailSprite", "Arial", 22);
			//������ʾ��Ŀ��size  
			auto textureSprite = seekByName(singleRootNode, "tail_tex_sprite");
			Size size = CCSizeMake(textureSprite->getContentSize().width * textureSprite->getScaleX(), textureSprite->getContentSize().height  * textureSprite->getScaleY());
			tailSpriteList = DropDownList::create(initLabel, size, tailSpriteNames.size() + 1);
			tailSpriteList->setPosition(textureSprite->getPositionX() - size.width / 2, textureSprite->getPositionY() - size.height / 2);
			tailSpriteList->setSwallowsTouches(true);
			textureSprite->getParent()->addChild(tailSpriteList, textureSprite->getLocalZOrder());

			////���ݶ���������png�ļ������֣���������Ӧ��label
			std::vector<std::string>::iterator texItor = tailSpriteNames.begin();
			while (texItor != tailSpriteNames.end()) {
				auto label = LabelTTF::create((*texItor), "Arial", 22);
				tailSpriteList->addLabel(label);
				texItor++;
			}
			tailSpriteList->setName("tailSpriteList");
			tailSpriteList->setItemCallBack(CC_CALLBACK_2(ParticleUiView::onDropDownList, this, tailSpriteList));

			if (tailSpriteNames.size() > 0) {
				tailSpriteList->setSelectedIndexByString(tailSpriteNames.at(0));
			}
		}
		// ��ѡ��
		if (nowEditingSignalPar->_tailPro.isActive) {
			((CheckBox*)seekByName(singleRootNode, "tail_active_checkbox"))->setSelectedState(true);
		}
		else {
			((CheckBox*)seekByName(singleRootNode, "tail_active_checkbox"))->setSelectedState(false);
		}

		tailSpriteList->setSelectedIndexByString(nowEditingSignalPar->_tailPro.tailName);

		char fadeStr[20];
		sprintf(fadeStr, "%.2f", nowEditingSignalPar->_tailPro.fade);
		((ExEditBox*)seekByName(singleRootNode, "tail_fade_input"))->setText(fadeStr);

		char minSegStr[20];
		sprintf(minSegStr, "%.2f", nowEditingSignalPar->_tailPro.minSeg);
		((ExEditBox*)seekByName(singleRootNode, "tail_minSeg_input"))->setText(minSegStr);

		char strokeStr[20];
		sprintf(strokeStr, "%.2f", nowEditingSignalPar->_tailPro.stroke);
		((ExEditBox*)seekByName(singleRootNode, "tail_stroke_input"))->setText(strokeStr);

		seekByName(singleRootNode, "tail_color_sprite")->setColor(nowEditingSignalPar->_tailPro.color);
		
	}
}

// ���� fireProLayer ����ʾ�����أ�ͬʱ�������е�slider�Ĵ�������������
void ParticleUiView::setFireProLayerVisible(bool visible) {
	// ����fireProLayer���е�slider�������� �� ������
	((ExSlider*)seekByName(singleRootNode, "maxParNum_Slider"))->setEnabled(visible);
	((ExSlider*)seekByName(singleRootNode, "duration_Slider"))->setEnabled(visible);
	((ExSlider*)seekByName(singleRootNode, "fireRate_Slider"))->setEnabled(visible);
	((ExSlider*)seekByName(singleRootNode, "preFire_Slider"))->setEnabled(visible);
	((ExSlider*)seekByName(singleRootNode, "fireDelay_Slider"))->setEnabled(visible);
	if (nowEditingSignalPar && nowEditingSignalPar->_fireArea.fAreaType == fireAreaType::rect && visible == true) {
		((ExSlider*)seekByName(singleRootNode, "rectModeOutWidth_Slider"))->setEnabled(visible);
		((ExSlider*)seekByName(singleRootNode, "rectModeInWidthSlider"))->setEnabled(visible);
		((ExSlider*)seekByName(singleRootNode, "rectModeOutHeight_Slider"))->setEnabled(visible);
		((ExSlider*)seekByName(singleRootNode, "rectModeInHeightSlider"))->setEnabled(visible);
	}
	if (nowEditingSignalPar && nowEditingSignalPar->_fireArea.fAreaType == fireAreaType::circle && visible == true) {
		((ExSlider*)seekByName(singleRootNode, "circleModeOutRadius_Slider"))->setEnabled(visible);
		((ExSlider*)seekByName(singleRootNode, "circleModeInRadiusSlider"))->setEnabled(visible);
	}
	if (nowEditingSignalPar && nowEditingSignalPar->_fireArea.fAreaType == fireAreaType::polygon && visible == true) {
		((ExSlider*)seekByName(singleRootNode, "edgeFireWidth_Slider"))->setEnabled(visible);
	}
	if (nowEditingSignalPar && nowEditingSignalPar->_fireArea.fAreaType == fireAreaType::lines && visible == true) {
		((ExSlider*)seekByName(singleRootNode, "edgeFireWidth_Slider"))->setEnabled(visible);
	}

	if(!visible){
		((ExSlider*)seekByName(singleRootNode, "rectModeOutWidth_Slider"))->setEnabled(visible);
		((ExSlider*)seekByName(singleRootNode, "rectModeInWidthSlider"))->setEnabled(visible);
		((ExSlider*)seekByName(singleRootNode, "rectModeOutHeight_Slider"))->setEnabled(visible);
		((ExSlider*)seekByName(singleRootNode, "rectModeInHeightSlider"))->setEnabled(visible);
		((ExSlider*)seekByName(singleRootNode, "circleModeOutRadius_Slider"))->setEnabled(visible);
		((ExSlider*)seekByName(singleRootNode, "circleModeInRadiusSlider"))->setEnabled(visible);
		((ExSlider*)seekByName(singleRootNode, "edgeFireWidth_Slider"))->setEnabled(visible);
	}

	seekByName(singleRootNode, "fireProLayer")->setVisible(visible);
	seekByName(singleRootNode, "fireProLayer")->setTouchEnabled(visible);

}


// ��ʾ ���ӷ��俪ʼʱ���������
void ParticleUiView::showFireStartProValuePanel(singleProType proType, emitterPropertyType type, bool isInitValue /*= false*/ ) {
	nowSelectedSingleProType = proType;

	seekByName(singleRootNode, "fireStartProValuePanel")->setVisible(true);
	seekByName(singleRootNode, "titleTypePanel")->setVisible(true);
	CCDictionary* namesXML = CCDictionary::createWithContentsOfFile("singleProTypeStrings.xml");
	int index = (int)proType;
	char key[20];
	sprintf(key, "name%d", index);
	auto nameKey = namesXML->valueForKey(key);
	std::string name = nameKey->getCString();

	((Text*)seekByName(singleRootNode, "titleWord_fireStart"))->setString(name);
	((Text*)seekByName(singleRootNode, "titleWord_fireStart"))->setColor(Color3B(255, 255, 0));

	emitterPropertyType targetType = type;
	//emitterVarietyValue nowEditingEmitterVarietyValue;
	
	if (nowSelectedSingleProType == singleProType::life_fireStart) {
		nowEditingEmitterVarietyValue = &nowEditingSignalPar->_life;
	}
	else if (nowSelectedSingleProType == singleProType::speed_fireStart) {
		nowEditingEmitterVarietyValue = &nowEditingSignalPar->_startMoveSpeed;
	}
	else if (nowSelectedSingleProType == singleProType::acc_fireStart) {
		nowEditingEmitterVarietyValue = &nowEditingSignalPar->_startMoveAcc;
	}
	else if (nowSelectedSingleProType == singleProType::moveAngle_fireStart) {
		nowEditingEmitterVarietyValue = &nowEditingSignalPar->_startMoveAngle;
	}
	else if (nowSelectedSingleProType == singleProType::moveAngleSpeed_fireStart) {
		nowEditingEmitterVarietyValue = &nowEditingSignalPar->_startMoveAngleSpeed;
	}
	else if (nowSelectedSingleProType == singleProType::gravityX_fireStart) {
		nowEditingEmitterVarietyValue = &nowEditingSignalPar->_startGravityX;
	}
	else if (nowSelectedSingleProType == singleProType::gravityY_fireStart) {
		nowEditingEmitterVarietyValue = &nowEditingSignalPar->_startGravityY;
	}
	else if (nowSelectedSingleProType == singleProType::size_fireStart) {
		nowEditingEmitterVarietyValue = &nowEditingSignalPar->_startSize;
	}
	else if (nowSelectedSingleProType == singleProType::rotation_fireStart) {
		nowEditingEmitterVarietyValue = &nowEditingSignalPar->_startRotation;
	}
	else if (nowSelectedSingleProType == singleProType::rotationSpeed_fireStart) {
		nowEditingEmitterVarietyValue = &nowEditingSignalPar->_startRotationSpeed;
	}
	else if (nowSelectedSingleProType == singleProType::skewX_fireStart) {
		nowEditingEmitterVarietyValue = &nowEditingSignalPar->_startSkewX;
	}
	else if (nowSelectedSingleProType == singleProType::skewXSpeed_fireStart) {
		nowEditingEmitterVarietyValue = &nowEditingSignalPar->_startSkewXSpeed;
	}
	else if (nowSelectedSingleProType == singleProType::skewY_fireStart) {
		nowEditingEmitterVarietyValue = &nowEditingSignalPar->_startSkewY;
	}
	else if (nowSelectedSingleProType == singleProType::skewYSpeed_fireStart) {
		nowEditingEmitterVarietyValue = &nowEditingSignalPar->_startSkewYSpeed;
	}
	else if (nowSelectedSingleProType == singleProType::alpha_fireStart) {
		nowEditingEmitterVarietyValue = &nowEditingSignalPar->_startAlpha;
	}
	/*else if (nowSelectedSingleProType == singleProType::color_fireStart) {
		targetEmitterPro = nowEditingSignalPar->_startColor;
	}*/
	
	if (isInitValue) {
		
		/*if (nowEditingEmitterVarietyValue->pType == emitterPropertyType::none) {
			nowEditingEmitterVarietyValue->pType = emitterPropertyType::oneConstant;
		}*/
		targetType = nowEditingEmitterVarietyValue->pType;
	}
	else {
		nowEditingEmitterVarietyValue->pType = targetType;
	}

	if (targetType == emitterPropertyType::oneConstant) { 
		seekByName(singleRootNode, "oneValuePanel")->setTouchEnabled(false);
		seekByName(singleRootNode, "oneValuePanel")->setSwallowTouches(true);  
		seekByName(singleRootNode, "oneValuePanel")->setVisible(true);
		
		seekByName(singleRootNode, "randTwoPanel")->setVisible(false);
		seekByName(singleRootNode, "moreValuePanel")->setVisible(false);
		seekByName(singleRootNode, "curveValuePanel_fireStart")->setVisible(false);

		((ExSlider*)seekByName(singleRootNode, "oneValue_Slider"))->setEnabled(true);
		((ExSlider*)seekByName(singleRootNode, "randValue1_Slider"))->setEnabled(false);
		((ExSlider*)seekByName(singleRootNode, "randValue2_Slider"))->setEnabled(false);

		fireStartProValueType->onClose();
		fireStartProValueType->setSelectedIndex(0);

		// ��ʼ��ֵ
		if (isInitValue) {
			((ExSlider*)seekByName(singleRootNode, "oneValue_Slider"))->setMaximumValueWithoutSetValue(100);
			((ExSlider*)seekByName(singleRootNode, "oneValue_Slider"))->setMinimumValueWithoutSetValue(0);
		}
		setSliderValueWithMaxMin(*((ExSlider*)seekByName(singleRootNode, "oneValue_Slider")), nowEditingEmitterVarietyValue->constValue);

		this->hideMoreConstantWidgetVec();
		this->hideFireStartCurvePoints();
	}
	else if (targetType == emitterPropertyType::randBetweenTwoConst) {
		seekByName(singleRootNode, "oneValuePanel")->setVisible(false);
		seekByName(singleRootNode, "oneValuePanel")->setTouchEnabled(false);
		seekByName(singleRootNode, "randTwoPanel")->setVisible(true);
		seekByName(singleRootNode, "moreValuePanel")->setVisible(false);
		seekByName(singleRootNode, "curveValuePanel_fireStart")->setVisible(false);

		((ExSlider*)seekByName(singleRootNode, "oneValue_Slider"))->setEnabled(false);
		((ExSlider*)seekByName(singleRootNode, "randValue1_Slider"))->setEnabled(true);
		((ExSlider*)seekByName(singleRootNode, "randValue2_Slider"))->setEnabled(true);

		fireStartProValueType->onClose();
		fireStartProValueType->setSelectedIndex(1);

		// ��ʼ��ֵ
		if (isInitValue) {
			((ExSlider*)seekByName(singleRootNode, "randValue1_Slider"))->setMaximumValueWithoutSetValue(100);
			((ExSlider*)seekByName(singleRootNode, "randValue1_Slider"))->setMinimumValueWithoutSetValue(0);
			((ExSlider*)seekByName(singleRootNode, "randValue2_Slider"))->setMaximumValueWithoutSetValue(100);
			((ExSlider*)seekByName(singleRootNode, "randValue2_Slider"))->setMinimumValueWithoutSetValue(0);
		}
		setSliderValueWithMaxMin(*((ExSlider*)seekByName(singleRootNode, "randValue1_Slider")), nowEditingEmitterVarietyValue->randConst1);
		setSliderValueWithMaxMin(*((ExSlider*)seekByName(singleRootNode, "randValue2_Slider")), nowEditingEmitterVarietyValue->randConst2);

		this->hideMoreConstantWidgetVec();
		this->hideFireStartCurvePoints();
	}
	else if (targetType == emitterPropertyType::moreConstant) {
		seekByName(singleRootNode, "oneValuePanel")->setVisible(false);
		seekByName(singleRootNode, "oneValuePanel")->setTouchEnabled(false);
		seekByName(singleRootNode, "randTwoPanel")->setVisible(false);
		seekByName(singleRootNode, "moreValuePanel")->setVisible(true);
		seekByName(singleRootNode, "curveValuePanel_fireStart")->setVisible(false);

		((ExSlider*)seekByName(singleRootNode, "oneValue_Slider"))->setEnabled(false);
		((ExSlider*)seekByName(singleRootNode, "randValue1_Slider"))->setEnabled(false);
		((ExSlider*)seekByName(singleRootNode, "randValue2_Slider"))->setEnabled(false);

		fireStartProValueType->onClose();
		fireStartProValueType->setSelectedIndex(2);

		// ��ʼ��ֵ
		((ExEditBox*)seekByName(singleRootNode, "moreValue_addInput"))->setText("0.0");
		this->hideMoreConstantWidgetVec();
		this->showMoreConstantWidgetVec(nowEditingEmitterVarietyValue->constValues);

		this->hideFireStartCurvePoints();
	}
	else if (targetType == emitterPropertyType::curve) {
		seekByName(singleRootNode, "oneValuePanel")->setVisible(false);
		seekByName(singleRootNode, "oneValuePanel")->setTouchEnabled(false);
		seekByName(singleRootNode, "randTwoPanel")->setVisible(false);
		seekByName(singleRootNode, "moreValuePanel")->setVisible(false);
		seekByName(singleRootNode, "curveValuePanel_fireStart")->setVisible(true);

		((ExSlider*)seekByName(singleRootNode, "oneValue_Slider"))->setEnabled(false);
		((ExSlider*)seekByName(singleRootNode, "randValue1_Slider"))->setEnabled(false);
		((ExSlider*)seekByName(singleRootNode, "randValue2_Slider"))->setEnabled(false);

		fireStartProValueType->onClose();
		fireStartProValueType->setSelectedIndex(3);

		this->hideMoreConstantWidgetVec();

		// ��ʼ��ֵ
		if (nowEditingEmitterVarietyValue->curvePoints.size() == 0) {
			nowEditingEmitterVarietyValue->curvePoints.push_back(Vec3(0, 0, 0));
			nowEditingEmitterVarietyValue->curvePoints.push_back(Vec3(1, 10, 0));
		}
		 
		this->hideFireStartCurvePoints();
		this->showFireStartCurvePoints();
	}


}
void ParticleUiView::hideFireStartProValuePanel() {
	nowSelectedSingleProType = singleProType::singleProTypeNone;
	seekByName(singleRootNode, "fireStartProValuePanel")->setVisible(false);
	seekByName(singleRootNode, "oneValuePanel")->setVisible(false);
	seekByName(singleRootNode, "randTwoPanel")->setVisible(false);
	seekByName(singleRootNode, "moreValuePanel")->setVisible(false);
	seekByName(singleRootNode, "curveValuePanel_fireStart")->setVisible(false);

	((ExSlider*)seekByName(singleRootNode, "oneValue_Slider"))->setEnabled(false);
	((ExSlider*)seekByName(singleRootNode, "randValue1_Slider"))->setEnabled(false);
	((ExSlider*)seekByName(singleRootNode, "randValue2_Slider"))->setEnabled(false);

	nowEditingEmitterVarietyValue = nullptr;
	this->hideMoreConstantWidgetVec();
	this->hideFireStartCurvePoints();

	this->hideFireStartProColorValuePanel();

	seekByName(singleRootNode, "fireStartBtn_color")->setColor(Color3B(255, 255, 255));
	this->hideSetColorPanel();
}

// ��ʾ������ ����ʱ���� ��ɫ ���Ե�ֵ�༭��� 
void ParticleUiView::showFireStartProColorValuePanel(singleProType proType, emitterPropertyType type, bool isInitValue /*= false*/) {
	nowEditingEmitterColorValue = &nowEditingSignalPar->_startColor;
	
	if (!randColorLine) {
		auto showRandColorLine = seekByName(singleRootNode, "randColorLine");
		randColorLine = ShaderSprite::create("randColorLineShader.png");
		randColorLine->setShaderFile("randColorLine.vsh", "randColorLine.fsh");
		randColorLine->setPosition(Vec2(showRandColorLine->getContentSize().width*0.5, showRandColorLine->getContentSize().height*0.5));

		Color3B color1 = nowEditingEmitterColorValue->randColor1;
		Color3B color2 = nowEditingEmitterColorValue->randColor2;

		randColorLine->setVec3Arg("randColorLeft", Vec3(color1.r, color1.g, color1.b));
		randColorLine->setVec3Arg("randColorRight", Vec3(color2.r, color2.g, color2.b));

		showRandColorLine->addChild(randColorLine);
	}
	else {
		Color3B color1 = nowEditingEmitterColorValue->randColor1;
		Color3B color2 = nowEditingEmitterColorValue->randColor2;

		randColorLine->setVec3Arg("randColorLeft", Vec3(color1.r, color1.g, color1.b));
		randColorLine->setVec3Arg("randColorRight", Vec3(color2.r, color2.g, color2.b));
	}
	


	//
	this->hideSetColorPanel();

	nowSelectedSingleProType = proType;

	seekByName(singleRootNode, "fireStartProValuePanel")->setVisible(true);
	seekByName(singleRootNode, "titleTypePanel")->setVisible(true);
	CCDictionary* namesXML = CCDictionary::createWithContentsOfFile("singleProTypeStrings.xml");
	int index = (int)proType;
	char key[20];
	sprintf(key, "name%d", index);
	auto nameKey = namesXML->valueForKey(key);
	std::string name = nameKey->getCString();

	((Text*)seekByName(singleRootNode, "titleWord_fireStart"))->setString(name);
	((Text*)seekByName(singleRootNode, "titleWord_fireStart"))->setColor(Color3B(255, 255, 0));

	emitterPropertyType targetType = type;

	nowEditingEmitterVarietyValue = nullptr;
	

	if (isInitValue) {
		targetType = nowEditingEmitterColorValue->pType;
	}
	else {
		nowEditingEmitterColorValue->pType = targetType;
	}

	if (targetType == emitterPropertyType::oneConstant) {
		// oneColorValuePanel �����壬������ ��ui�༭�� �������� ѡ��

		seekByName(singleRootNode, "oneColorValuePanel")->setTouchEnabled(false);
		seekByName(singleRootNode, "oneColorValuePanel")->setVisible(true);
		seekByName(singleRootNode, "randTwoColorPanel")->setVisible(false);
		seekByName(singleRootNode, "moreColorValuePanel")->setVisible(false);
		seekByName(singleRootNode, "curveColorValuePanel_fireStart")->setVisible(false);

		seekByName(singleRootNode, "oneColorValueBtn")->setColor(nowEditingEmitterColorValue->constColor);

		seekByName(singleRootNode, "oneColorValueBtn")->setEnabled(true);

		fireStartProValueType->onClose();
		fireStartProValueType->setSelectedIndex(0);

		this->hideFireStartCurveColors();
		this->hideMoreColorWidgetVec();
	}
	else if (targetType == emitterPropertyType::randBetweenTwoConst) {
		seekByName(singleRootNode, "oneColorValuePanel")->setVisible(false);
		seekByName(singleRootNode, "randTwoColorPanel")->setVisible(true);
		seekByName(singleRootNode, "moreColorValuePanel")->setVisible(false);
		seekByName(singleRootNode, "curveColorValuePanel_fireStart")->setVisible(false);

		seekByName(singleRootNode, "randColor1")->setColor(nowEditingEmitterColorValue->randColor1);
		seekByName(singleRootNode, "randColor2")->setColor(nowEditingEmitterColorValue->randColor2);

		fireStartProValueType->onClose();
		fireStartProValueType->setSelectedIndex(1);

		this->hideFireStartCurveColors();
		this->hideMoreColorWidgetVec();
	}
	else if (targetType == emitterPropertyType::moreConstant) {
		seekByName(singleRootNode, "oneColorValuePanel")->setVisible(false);
		seekByName(singleRootNode, "randTwoColorPanel")->setVisible(false);
		seekByName(singleRootNode, "moreColorValuePanel")->setVisible(true);
		seekByName(singleRootNode, "curveColorValuePanel_fireStart")->setVisible(false);

		fireStartProValueType->onClose();
		fireStartProValueType->setSelectedIndex(2);

		this->hideFireStartCurveColors();
		this->hideMoreColorWidgetVec();
		this->showMoreColorWidgetVec(nowEditingEmitterColorValue->constColors);
	}
	else if (targetType == emitterPropertyType::curve) {
		seekByName(singleRootNode, "oneColorValuePanel")->setVisible(false);
		seekByName(singleRootNode, "randTwoColorPanel")->setVisible(false);
		seekByName(singleRootNode, "moreColorValuePanel")->setVisible(false);
		seekByName(singleRootNode, "curveColorValuePanel_fireStart")->setVisible(true);

		fireStartProValueType->onClose();
		fireStartProValueType->setSelectedIndex(3);

		this->hideMoreColorWidgetVec();

		if (!fireStartCurveColorLine) {
			auto curveColorValueShowLine = seekByName(singleRootNode, "curveColorValueShowLine");
			fireStartCurveColorLine = ShaderSprite::create("curveColorLineShader.png");
			fireStartCurveColorLine->setShaderFile("curveColorLine.vsh", "curveColorLine.fsh");
			fireStartCurveColorLine->setPosition(Vec2(curveColorValueShowLine->getContentSize().width*0.5, curveColorValueShowLine->getContentSize().height*0.5));

			curveColorValueShowLine->addChild(fireStartCurveColorLine);
		}

		// ��ʼ��ֵ
		if (nowEditingEmitterColorValue->curveColors.size() == 0) {
			colorCurvePoint color1;
			color1.x = 0.2;
			color1.colorY = Color3B(255, 255, 255);
			colorCurvePoint color2;
			color2.x = 0.8;
			color2.colorY = Color3B(255, 0, 255);

			nowEditingEmitterColorValue->curveColors.push_back(color1);
			nowEditingEmitterColorValue->curveColors.push_back(color2);
		}

		this->hideFireStartCurveColors();
		this->showFireStartCurveColors();
	}
}
void ParticleUiView::hideFireStartProColorValuePanel() {
	nowSelectedSingleProType = singleProType::singleProTypeNone;
	seekByName(singleRootNode, "fireStartProValuePanel")->setVisible(false);
	seekByName(singleRootNode, "oneColorValuePanel")->setVisible(false);
	seekByName(singleRootNode, "randTwoColorPanel")->setVisible(false);
	seekByName(singleRootNode, "moreColorValuePanel")->setVisible(false);
	seekByName(singleRootNode, "curveColorValuePanel_fireStart")->setVisible(false);

	this->hideSetColorPanel();

	this->hideFireStartCurveColors();
	if (fireStartCurveColorLine) {
		if (fireStartCurveColorLine->getParent()) {
			fireStartCurveColorLine->removeFromParent();
		}
		fireStartCurveColorLine = nullptr;
	}
}
void ParticleUiView::refreshFireStartProColorValuePanel() {
	if (nowSelectedSingleProType == singleProType::color_fireStart) {
		if (nowEditingEmitterColorValue->pType == emitterPropertyType::oneConstant) {
			nowEditingEmitterColorValue->constColor = seekByName(singleRootNode, "oneColorValueBtn")->getColor();
		}
		else if (nowEditingEmitterColorValue->pType == emitterPropertyType::randBetweenTwoConst) {
			auto randColor1 = seekByName(singleRootNode, "randColor1");
			auto randColor2 = seekByName(singleRootNode, "randColor2");
			if (nowEditingColorNode == randColor1) {
				nowEditingEmitterColorValue->randColor1 = randColor1->getColor();
			}
			else if (nowEditingColorNode == randColor2) { 
				nowEditingEmitterColorValue->randColor2 = randColor2->getColor();
			}

			Color3B color1 = nowEditingEmitterColorValue->randColor1;
			Color3B color2 = nowEditingEmitterColorValue->randColor2;

			randColorLine->setVec3Arg("randColorLeft", Vec3(color1.r, color1.g, color1.b));
			randColorLine->setVec3Arg("randColorRight", Vec3(color2.r, color2.g, color2.b));
		}
		else if (nowEditingEmitterColorValue->pType == emitterPropertyType::moreConstant) {
			if (nowEditingEmitterColorValue->constColors.size() == moreColorWidgetVec.size()) {
				for (int i = 0; i < nowEditingEmitterColorValue->constColors.size(); ++i) {
					nowEditingEmitterColorValue->constColors.at(i) = moreColorWidgetVec.at(i)->getChildByTag(9527)->getColor();

				}

			}
		}
		else if (nowEditingEmitterColorValue->pType == emitterPropertyType::curve) {
			this->refreshFireStartCurveColors();
		}

	}
}

void ParticleUiView::showSetColorPanel(bool isShowRandColor /*= false*/) {
	isOpenSetColorPanel = true;
	
	seekByName(singleRootNode, "setColorPanel")->setVisible(true);
	auto showColorRect = seekByName(singleRootNode, "showColorRect");
	auto showColorLine = seekByName(singleRootNode, "showColorLine");
	

	if (isShowRandColor) {
		seekByName(singleRootNode, "randColorPanel")->setVisible(true); 
	}
	else {
		seekByName(singleRootNode, "randColorPanel")->setVisible(false);
	}

	// ��ʾ���ֵ
	if (isShowRandColor && nowEditingColorPoint) {
		char randColorR[20];
		sprintf(randColorR, "%.1f", nowEditingColorPoint->colorRand.x);
		((ExEditBox*)seekByName(singleRootNode, "randR_ColorInput"))->setText(randColorR);

		char randColorG[20];
		sprintf(randColorG, "%.1f", nowEditingColorPoint->colorRand.y);
		((ExEditBox*)seekByName(singleRootNode, "randG_ColorInput"))->setText(randColorG);

		char randColorB[20];
		sprintf(randColorB, "%.1f", nowEditingColorPoint->colorRand.z);
		((ExEditBox*)seekByName(singleRootNode, "randB_ColorInput"))->setText(randColorB);
	}

	// ������ɫ�� �� ��ɫ��
	if (!colorRect) {
		
		colorRect = ShaderSprite::create("colorRectShader.png");
		colorRect->setShaderFile("colorRect.vsh", "colorRect.fsh");
		colorRect->setPosition(Vec2(showColorRect->getContentSize().width*0.5 , showColorRect->getContentSize().height*0.5));
		//Color3B color = nowEditingEmitterColorValue->constColor;
		//colorRect->setVec3Arg("rightTopColor",Vec3(color.r, color.g, color.b)); 
		colorRect->setVec3Arg("rightTopColor", Vec3(255, 0, 0));
		showColorRect->addChild(colorRect,1);
	}
	if (!colorLine) { 
		
		colorLine = ShaderSprite::create("colorLineShader.png");
		colorLine->setShaderFile("colorLine.vsh", "colorLine.fsh");
		colorLine->setPosition(Vec2(showColorLine->getContentSize().width*0.5, showColorLine->getContentSize().height*0.5));

		showColorLine->addChild(colorLine,1); 
	}
	


}
void ParticleUiView::hideSetColorPanel() {
	isOpenSetColorPanel = false;
	seekByName(singleRootNode, "setColorPanel")->setVisible(false);
	nowEditingColorPoint = nullptr;
}



void ParticleUiView::setLineColor(float posY) {
	Color3B startColor;
	Color3B endColor;
	float startY;
	float endY;

	auto showColorLine = seekByName(singleRootNode, "showColorLine");
	float cSizeHeight = showColorLine->getContentSize().height;

	if (posY >= 5.0 / 6.0 * cSizeHeight) {
		startColor = LINE_COLOR_6;
		endColor = LINE_COLOR_7;
		startY = 5.0 / 6.0 * cSizeHeight;
		endY = 6.0 / 6.0 * cSizeHeight;
	}
	else if (posY >= 4.0 / 6.0 * cSizeHeight) {
		startColor = LINE_COLOR_5;
		endColor = LINE_COLOR_6;
		startY = 4.0 / 6.0 * cSizeHeight;
		endY = 5.0 / 6.0 * cSizeHeight;
	}
	else if (posY >= 3.0 / 6.0 * cSizeHeight) {
		startColor = LINE_COLOR_4;
		endColor = LINE_COLOR_5;
		startY = 3.0 / 6.0 * cSizeHeight;
		endY = 4.0 / 6.0 * cSizeHeight;
	}
	else if (posY >= 2.0 / 6.0 * cSizeHeight) {
		startColor = LINE_COLOR_3;
		endColor = LINE_COLOR_4;
		startY = 2.0 / 6.0 * cSizeHeight;
		endY = 3.0 / 6.0 * cSizeHeight;
	}
	else if (posY >= 1.0 / 6.0 * cSizeHeight) {
		startColor = LINE_COLOR_2;
		endColor = LINE_COLOR_3;
		startY = 1.0 / 6.0 * cSizeHeight;
		endY = 2.0 / 6.0 * cSizeHeight;
	}
	else if (posY >= 0.0 / 6.0 * cSizeHeight) {
		startColor = LINE_COLOR_1;
		endColor = LINE_COLOR_2;
		startY = 0.0 / 6.0 * cSizeHeight;
		endY = 1.0 / 6.0 * cSizeHeight;
	}

	Vec3 nowColor = Vec3(startColor.r + (posY - startY) / (endY - startY) * (endColor.r - startColor.r),
		startColor.g + (posY - startY) / (endY - startY) * (endColor.g - startColor.g),
		startColor.b + (posY - startY) / (endY - startY) * (endColor.b - startColor.b));

	lineColor = Vec3( roundf(nowColor.x) , roundf(nowColor.y), roundf(nowColor.z));

	colorRect->setVec3Arg("rightTopColor", Vec3(lineColor.x, lineColor.y, lineColor.z));

	auto showRectColor = seekByName(singleRootNode, "showColorRect");

	auto rectColorPtr = seekByName(singleRootNode, "showColorRectPointer");
	Vec2 rectColorPtrPos = rectColorPtr->getPosition();
	Vec2 offsetPos = Vec2(rectColorPtrPos.x - showRectColor->getPositionX() + showRectColor->getContentSize().width/2 , rectColorPtrPos.y - showRectColor->getPositionY() + showRectColor->getContentSize().height / 2);

	this->setRectColor(offsetPos);
}
void ParticleUiView::setRectColor(Vec2 pos, Color3B* realColor) {
	auto showRectColor = seekByName(singleRootNode, "showColorRect");
	Vec2 realPos = Vec2(pos.x / showRectColor->getContentSize().width , (showRectColor->getContentSize().height - pos.y) / showRectColor->getContentSize().height);

	Vec3 nowColorX = Vec3(255.0 + realPos.x * (lineColor.x - 255.0), 255.0 + realPos.x * (lineColor.y - 255.0), 255.0 + realPos.x * (lineColor.z - 255.0));
	Vec3 nowColor = Vec3(nowColorX.x + realPos.y * (0.0 - nowColorX.x), nowColorX.y + realPos.y * (0.0 - nowColorX.y), nowColorX.z + realPos.y * (0.0 - nowColorX.z));

	rectColor = nowColor;

	seekByName(singleRootNode, "nowColor")->setColor(Color3B(rectColor.x , rectColor.y , rectColor.z));

	Color3B colroTem;
	if (realColor) {
		colroTem = Color3B(realColor->r, realColor->g, realColor->b);
		rectColor = Vec3(realColor->r, realColor->g, realColor->b);
	}
	else {
		colroTem = Color3B(rectColor.x, rectColor.y, rectColor.z);
	}

	char colorR[20];
	sprintf(colorR, "%.1f" , (float)colroTem.r);
	((ExEditBox*)seekByName(singleRootNode, "R_ColorInput"))->setText(colorR);
	char colorG[20];
	sprintf(colorG, "%.1f", (float)colroTem.g);
	((ExEditBox*)seekByName(singleRootNode, "G_ColorInput"))->setText(colorG);
	char colorB[20];
	sprintf(colorB, "%.1f", (float)colroTem.b);
	((ExEditBox*)seekByName(singleRootNode, "B_ColorInput"))->setText(colorB);

}

// ���������rgbֵ������ rectPointer �� linePointer ��λ��
void ParticleUiView::setPointerPosByRGB(Color3B color) {
	// ԭ��:rgb ת hsb
	float hue;            // ɫ��
	float saturation;     // ���Ͷ�  �൱�� x ����
	float brightness;     // ����    �൱�� y ����
	
	float maxColor = max(max(color.r, color.g), color.b);
	float minColor = min(min(color.r, color.g), color.b);
	if (maxColor == minColor) { 
		hue = 0; // 0 ��
	}
	else if (maxColor == color.r) {
		if (color.g >= color.b) {
			hue = (60 * (color.g - color.b) / (maxColor - minColor)) + 0;
		}
		else if (color.g < color.b) {
			hue = (60 * (color.g - color.b) / (maxColor - minColor)) + 360;
		}
		while (hue < 0) {
			hue += 360;
		}
	}
	else if (maxColor == color.g) {
		hue = 60 * (color.b - color.r) / (maxColor - minColor) + 120;
	}
	else if (maxColor == color.b) {
		hue = 60 * (color.r - color.g) / (maxColor - minColor) + 240;
	}

	hue = floorf(hue);

	brightness = maxColor / 255.0;

	saturation = (maxColor - minColor) / maxColor;


	// ����linePointer��λ��
	float linePointerHeight;

	auto showColorLine = seekByName(singleRootNode, "showColorLine");
	Vec2 showColorLinePos = showColorLine->getPosition();
	float showColorLineHeight = showColorLine->getContentSize().height;

	linePointerHeight = hue / 360 * showColorLineHeight;
	
	auto lineColorPtr = seekByName(singleRootNode, "showColorLinePointer");
	lineColorPtr->setPositionY(showColorLinePos.y - showColorLineHeight/2 + linePointerHeight);
	
	this->setLineColor(linePointerHeight );

	
	// ����rectPointer��λ��
	auto showRectColor = seekByName(singleRootNode, "showColorRect");
	Vec2 showRectColorPos = showRectColor->getPosition();
	Size showRectSize = showRectColor->getContentSize();
	Vec2 rectPointerPos = Vec2((saturation*showRectSize.width), (brightness*showRectSize.height));

	auto rectColorPtr = seekByName(singleRootNode, "showColorRectPointer");
	rectColorPtr->setPosition(Vec2(showRectColorPos.x - showRectSize.width / 2 + rectPointerPos.x, showRectColorPos.y - showRectSize.height / 2 + rectPointerPos.y));
	this->setRectColor(rectPointerPos , &color);
}

////---- �����ɫ֮�е������ɫ
void ParticleUiView::showMoreColorWidgetVec(std::vector<Color3B>& vec) {
	auto itor = vec.begin();
	int index = 0;
	while (itor != vec.end()) {
		this->addMoreColorWidgetVec((*itor));

		index++;
		itor++;
	}
}
void ParticleUiView::hideMoreColorWidgetVec() {
	auto itor = moreColorWidgetVec.begin();
	while (itor != moreColorWidgetVec.end()) {
		(*itor)->removeFromParent();

		itor++;
	}
	moreColorWidgetVec.clear();
}
void ParticleUiView::addMoreColorWidgetVec(Color3B color) {
	auto scrollView = (ScrollView*)seekByName(singleRootNode, "moreColorValueScrollView");
	Size scrollCSize = scrollView->getContentSize();
	scrollView->setTouchEnabled(true);
	scrollView->setBounceEnabled(true);



	auto item = ImageView::create("moreValueOneItem.png");
	item->setColor(Color3B(255, 134, 53));
	Size cSize = item->getContentSize();

	Size innerSize = Size(scrollCSize.width, (moreColorWidgetVec.size() + 1) * cSize.height);
	if (innerSize.height < scrollCSize.height) {
		innerSize.height = scrollCSize.height;
	}
	else {
		for (int i = 0; i < moreColorWidgetVec.size(); ++i) {
			moreColorWidgetVec.at(i)->setPositionY(innerSize.height - (i + 1) * cSize.height);
		}

	}
	scrollView->setInnerContainerSize(innerSize);



	item->setAnchorPoint(Vec2(0, 0));
	item->setPosition(Vec2(0, innerSize.height - (moreColorWidgetVec.size() + 1) * cSize.height));

	scrollView->addChild(item);
	scrollView->scrollToBottom(0.5, true);


	// ��ɫ ��ť
	auto colorBtn = ui::Button::create(
		"colorSprite.png",
		"colorSprite.png");
	
	colorBtn->setScaleX(80 / colorBtn->getContentSize().width);
	colorBtn->setScaleY(20 / colorBtn->getContentSize().height);
	colorBtn->setPosition(Vec2(55, 15));
	colorBtn->setSwallowTouches(false);
	colorBtn->setTag(9527);
	char name[20];
	sprintf(name, "%d", moreColorWidgetVec.size());
	colorBtn->setName(name);
	colorBtn->setColor(color);
	item->addChild(colorBtn);
	colorBtn->addTouchEventListener([this](Ref* pSender, Widget::TouchEventType eType) {
		if (eType == Widget::TouchEventType::ENDED) {
			nowEditingColorNode = (Widget*)pSender;
			int index = std::stoi( nowEditingColorNode->getName() );
			this->showSetColorPanel();
			this->setPointerPosByRGB(nowEditingColorNode->getColor());

		}
	});


	// ��ť
	auto subBtn = ImageView::create("sub.png");
	subBtn->setTouchEnabled(true);
	subBtn->setPosition(Vec2(123, 15));
	item->addChild(subBtn);

	subBtn->addTouchEventListener([this, item, scrollView](Ref* pSender, Widget::TouchEventType eType) {
		if (eType == Widget::TouchEventType::ENDED) {
			this->hideSetColorPanel();
			
			auto itor = this->moreColorWidgetVec.begin();
			int index = 0;
			while (itor != this->moreColorWidgetVec.end()) {
				if ((*itor) == item) {

					Size innerSize = Size(item->getContentSize().width, (this->moreColorWidgetVec.size() - 1) * item->getContentSize().height);
					if (innerSize.height < scrollView->getContentSize().height) {
						innerSize.height = scrollView->getContentSize().height;
					}
					scrollView->setInnerContainerSize(innerSize);

					float cHeight = item->getContentSize().height;
					(*itor)->removeFromParent();
					this->moreColorWidgetVec.erase(itor);

					auto itorTem = nowEditingEmitterColorValue->constColors.begin() + index;
					nowEditingEmitterColorValue->constColors.erase(itorTem);

					for (int i = 0; i < this->moreColorWidgetVec.size(); ++i) {
						this->moreColorWidgetVec.at(i)->setPositionY(innerSize.height - (i + 1) * cHeight);
						char name[20];
						sprintf(name, "%d", i);
						this->moreColorWidgetVec.at(i)->getChildByTag(9527)->setName(name);
					}


					break;
				}
				itor++;
				index++;
			}

		}
	});

	moreColorWidgetVec.push_back(item);
}

// -------------- ��ʾ�����ֵ��ui�б�
void ParticleUiView::showMoreConstantWidgetVec(std::vector<float>& vec) {
	auto itor = vec.begin();
	int index = 0;
	while (itor != vec.end()) {
		this->addMoreConstantWidgetVec((*itor));

		index++;
		itor++;
	}
}

void ParticleUiView::hideMoreConstantWidgetVec() {
	auto itor = moreConstantWidgetVec.begin();
	while (itor != moreConstantWidgetVec.end()) {
		(*itor)->removeFromParent();

		itor++;
	}
	moreConstantWidgetVec.clear();
}

void ParticleUiView::addMoreConstantWidgetVec(float value) {
	auto scrollView = (ScrollView*)seekByName(singleRootNode, "moreValueScrollView");
	Size scrollCSize = scrollView->getContentSize();
	scrollView->setTouchEnabled(true);
	scrollView->setBounceEnabled(true);
	
	

	auto item = ImageView::create("moreValueOneItem.png"); 
	item->setColor(Color3B(255,134,53));
	Size cSize = item->getContentSize();

	Size innerSize = Size(scrollCSize.width, (moreConstantWidgetVec.size() + 1) * cSize.height);
	if (innerSize.height < scrollCSize.height) {
		innerSize.height = scrollCSize.height;
	}
	else {
		for (int i = 0; i < moreConstantWidgetVec.size(); ++i) {
			moreConstantWidgetVec.at(i)->setPositionY(innerSize.height - (i+1) * cSize.height);
		}
	}
	scrollView->setInnerContainerSize(innerSize);


	 
	item->setAnchorPoint(Vec2(0, 0));
	item->setPosition(Vec2(0, innerSize.height - (moreConstantWidgetVec.size() + 1) * cSize.height));
	
	scrollView->addChild(item);
	scrollView->scrollToBottom(0.5, true);
	

	// �ı�
	auto textEditBox = ExEditBox::create(Size(50, 28), ui::Scale9Sprite::create("editBg.png"));

	textEditBox->setAnchorPoint(Vec2(0.5,0.5));
	textEditBox->setPosition(Vec2(55,15));
	textEditBox->setInputMode(ExEditBox::InputMode::NUMERIC);
	textEditBox->setFontColor(Color3B::BLACK);
	textEditBox->setFontSize(20);
	char name[20];
	textEditBox->setTag(9527);
	sprintf(name, "moreConstant_%d", moreConstantWidgetVec.size());
	textEditBox->setName(name);
	textEditBox->setDelegate(this);
	item->addChild(textEditBox);

	char tex[20];
	sprintf(tex, "%.2f", value);
	textEditBox->setText(tex);

	// ��ť
	auto subBtn = ImageView::create("sub.png");
	subBtn->setTouchEnabled(true);
	subBtn->setPosition(Vec2(123,15));
	item->addChild(subBtn); 

	subBtn->addTouchEventListener([this, item , scrollView](Ref* pSender, Widget::TouchEventType eType) {
		if (eType == Widget::TouchEventType::ENDED) {
			auto itor = this->moreConstantWidgetVec.begin();
			int index = 0;
			while (itor != this->moreConstantWidgetVec.end()) {
				if ((*itor) == item) {
					
					Size innerSize = Size(item->getContentSize().width , (this->moreConstantWidgetVec.size() - 1) * item->getContentSize().height);
					if (innerSize.height < scrollView->getContentSize().height) {
						innerSize.height = scrollView->getContentSize().height;
					}
					scrollView->setInnerContainerSize(innerSize);

					float cHeight = item->getContentSize().height;
					(*itor)->removeFromParent();
					this->moreConstantWidgetVec.erase(itor);
					
					auto itorTem = nowEditingEmitterVarietyValue->constValues.begin() + index;
					nowEditingEmitterVarietyValue->constValues.erase(itorTem);

					for (int i = 0; i < this->moreConstantWidgetVec.size(); ++i) {
						this->moreConstantWidgetVec.at(i)->setPositionY(innerSize.height - (i + 1) * cHeight);
						char name[20];
						sprintf(name, "moreConstant_%d", i);
						this->moreConstantWidgetVec.at(i)->getChildByTag(9527)->setName(name);
					}

					
					break;
				}
				itor++;
				index++;
			}

		}
	});

	moreConstantWidgetVec.push_back(item);

}

//////***************************** ����ʱ�������Ե� �������� ��� *****************************
void ParticleUiView::showFireStartCurvePoints() {
	isOpenFireStartCurvePanel = true;

	auto curvePoints = nowEditingEmitterVarietyValue->curvePoints; 
	auto itor = curvePoints.begin();
	
	auto curvePanel = seekByName(singleRootNode, "curveMap_fireStart"); 
	curvePanel->setSwallowTouches(true);
	Size curvePanelSize = curvePanel->getContentSize();
	Vec2 curvePanelPos = Vec2(curvePanelSize.width / 2, curvePanelSize.height / 2);
	// �ҵ����߶�
	fireStartCurvePointMaxHeight = 0;
	while (itor != curvePoints.end()) {
		if ( fabsf((*itor).y) > fireStartCurvePointMaxHeight) {
			fireStartCurvePointMaxHeight = (*itor).y * ((*itor).y / fabsf((*itor).y));
		}
		itor++;
	}
	// �ı�����
	char maxStr[20];
	char minStr[20];
	char durationStr[20];
	sprintf(maxStr, "%.2f", fireStartCurvePointMaxHeight);
	sprintf(minStr, "-%.2f", fireStartCurvePointMaxHeight);
	sprintf(durationStr, "%.2f", nowEditingSignalPar->_duration);
	((ExEditBox*)seekByName(singleRootNode, "topYInput_fireStart"))->setText(maxStr);
	((Text*)seekByName(singleRootNode, "downYWord_fireStart"))->setString(minStr);
	((Text*)seekByName(singleRootNode, "rightXWord_fireStart"))->setString(durationStr);

	// �ӵ�
	itor = curvePoints.begin();
	while (itor != curvePoints.end()) {
		Vec2 inPanelPos = Vec2(curvePanelPos.x - curvePanelSize.width/2 + (*itor).x * curvePanelSize.width , curvePanelPos.y + ((*itor).y / fireStartCurvePointMaxHeight) * curvePanelSize.height/2 );
		
		// ��һ�� ����������еĵ� ��ť
		auto curvePoint = ui::Button::create(
			"coord_point1.png",
			"coord_point2.png");

		curvePoint->setPosition(inPanelPos);
		fireStartCurvePointBtnVec.push_back(curvePoint);
		curvePoint->setSwallowTouches(false);
		curvePanel->addChild(curvePoint);
		curvePoint->addTouchEventListener(CC_CALLBACK_2(ParticleUiView::fireStartCurvePointCall, this));

		itor++;
	}

	// ���� �����ӵ���� �� 
	
	for (int i = 0; i < fireStartCurvePointBtnVec.size()-1; ++i) {
		auto lineSprite = Sprite::create("coord_line.png");
		curvePanel->addChild(lineSprite);

		Vec2 point1 = fireStartCurvePointBtnVec.at(i)->getPosition();
		Vec2 point2 = fireStartCurvePointBtnVec.at(i+1)->getPosition();
		Vec2 realPos = Vec2( point1.x + (point2.x - point1.x)/2 , point1.y + (point2.y - point1.y) / 2);

		float angle = Vec2(point1.x - point2.x, point1.y - point2.y).getAngle() / P_PI * 180;
		float dis = point1.getDistance(point2);

		lineSprite->setScaleX(dis / lineSprite->getContentSize().width);
		lineSprite->setScaleY(1 / lineSprite->getContentSize().height);
		lineSprite->setPosition(realPos);
		lineSprite->setRotation(-angle);
		
		fireStartCurveLines.push_back(lineSprite);

	}

	// �� ���ֵ�� �༭��
	for (int i = 0; i < fireStartCurvePointBtnVec.size(); ++i) {
		Vec2 point = fireStartCurvePointBtnVec.at(i)->getPosition();
		// �����ֵ�ı༭��
		auto editBox = ExEditBox::create(Size(35, 25), ui::Scale9Sprite::create("editBg.png"));
		editBox->setAnchorPoint(Vec2(0.5, 0));
		editBox->setPosition(Vec2(point.x, curvePanelSize.height + 5));
		editBox->setInputMode(ExEditBox::InputMode::NUMERIC);
		editBox->setFontColor(Color3B::BLACK);
		editBox->setFontSize(20);
		char name[50];
		sprintf(name , "fireStartCurveRandEditBox_%d" , i);
		editBox->setName(name);
		char value[20];
		sprintf(value, "%.2f", curvePoints.at(i).z);
		editBox->setText(value);
		editBox->setDelegate(this);
		curvePanel->addChild(editBox);

		fireStartCurveRandEditBoxVec.push_back(editBox);
	}

	// �� ���� ���ֵ�༭�� ����
	for (int i = 0; i < fireStartCurvePointBtnVec.size(); ++i) {
		auto lineSprite = Sprite::create("coord_xu_line.png");
		curvePanel->addChild(lineSprite);

		
		Vec2 point = fireStartCurvePointBtnVec.at(i)->getPosition();
		float angle = 90;
		float dis = curvePanelSize.height - point.y;

		Vec2 realPos = Vec2(point.x , point.y + dis / 2);

		lineSprite->setScaleX(dis / lineSprite->getContentSize().width);
		lineSprite->setScaleY(2 / lineSprite->getContentSize().height);
		lineSprite->setColor(Color3B(255,80,0));
		lineSprite->setPosition(realPos);
		lineSprite->setRotation(-angle);

		fireStartCurveRandLines.push_back(lineSprite);
	}

}

void ParticleUiView::hideFireStartCurvePoints() {
	isOpenFireStartCurvePanel = false;

	// ɾ�㰴ť
	auto itor = fireStartCurvePointBtnVec.begin();
	while (itor != fireStartCurvePointBtnVec.end()) {
		(*itor)->removeFromParent();

		itor++;
	}
	fireStartCurvePointBtnVec.clear();

	// ɾ ���ӵ����
	auto itor2 = fireStartCurveLines.begin();
	while (itor2 != fireStartCurveLines.end()) {
		(*itor2)->removeFromParent();

		itor2++;
	}
	fireStartCurveLines.clear();

	// ɾ���ֵ�ı༭��
	auto itor3 = fireStartCurveRandEditBoxVec.begin();
	while (itor3 != fireStartCurveRandEditBoxVec.end()) {
		(*itor3)->removeFromParent();

		itor3++;
	}
	fireStartCurveRandEditBoxVec.clear();

	// ɾ���� ���ֵ�༭��� ��
	auto itor4 = fireStartCurveRandLines.begin();
	while (itor4 != fireStartCurveRandLines.end()) {
		(*itor4)->removeFromParent();

		itor4++;
	}
	fireStartCurveRandLines.clear();
}

void ParticleUiView::addFireStartCurvePoints(Vec2 pos) {
	auto curvePanel = seekByName(singleRootNode, "curveMap_fireStart");
	Size curvePanelSize = seekByName(singleRootNode, "curveMap_fireStart")->getContentSize();
	Vec2 curvePanelPos = curvePanel->convertToWorldSpace(Vec2::ZERO);

	Vec2 inPanelPos = Vec2(pos.x - curvePanelPos.x  , pos.y - curvePanelPos.y );

	Vec2 value = Vec2(inPanelPos.x / curvePanelSize.width , (inPanelPos.y - curvePanelSize.height / 2) / (curvePanelSize.height/2) * fireStartCurvePointMaxHeight);
	
	// �ҵ���ȷ��λ�ò���
	int size = fireStartCurvePointBtnVec.size();
	for (int i = 1; i < size; ++i) {
		if (inPanelPos.x <= fireStartCurvePointBtnVec.at(i)->getPositionX()) {
			auto curvePoint = ui::Button::create(
				"coord_point1.png",
				"coord_point2.png");

			curvePoint->setPosition(inPanelPos);
			curvePoint->setSwallowTouches(false);
			curvePanel->addChild(curvePoint);
			curvePoint->addTouchEventListener(CC_CALLBACK_2(ParticleUiView::fireStartCurvePointCall, this));
			
			fireStartCurvePointBtnVec.insert(fireStartCurvePointBtnVec.begin() + i ,curvePoint);
			nowEditingEmitterVarietyValue->curvePoints.insert(nowEditingEmitterVarietyValue->curvePoints.begin() + i , Vec3(value.x , value.y , 0));
			break;
		}

	}
	auto lineItor = fireStartCurveLines.begin();
	while (lineItor != fireStartCurveLines.end()) {
		(*lineItor)->removeFromParent();
		lineItor++;
	}
	fireStartCurveLines.clear();
	// ɾ���ֵ�ı༭��
	auto randEditBoxItor = fireStartCurveRandEditBoxVec.begin();
	while (randEditBoxItor != fireStartCurveRandEditBoxVec.end()) {
		(*randEditBoxItor)->removeFromParent();
		randEditBoxItor++;
	}
	fireStartCurveRandEditBoxVec.clear();

	// ɾ���� ���ֵ�༭��� ��
	auto randLineItor = fireStartCurveRandLines.begin();
	while (randLineItor != fireStartCurveRandLines.end()) {
		(*randLineItor)->removeFromParent();
		randLineItor++;
	}
	fireStartCurveRandLines.clear();

	// ����
	for (int i = 0; i < fireStartCurvePointBtnVec.size() - 1; ++i) {
		auto lineSprite = Sprite::create("coord_line.png");
		curvePanel->addChild(lineSprite);

		Vec2 point1 = fireStartCurvePointBtnVec.at(i)->getPosition();
		Vec2 point2 = fireStartCurvePointBtnVec.at(i + 1)->getPosition();
		Vec2 realPos = Vec2(point1.x + (point2.x - point1.x) / 2, point1.y + (point2.y - point1.y) / 2);

		float angle = Vec2(point1.x - point2.x, point1.y - point2.y).getAngle() / P_PI * 180;
		float dis = point1.getDistance(point2);

		lineSprite->setScaleX(dis / lineSprite->getContentSize().width);
		lineSprite->setScaleY(1 / lineSprite->getContentSize().height);
		lineSprite->setPosition(realPos);
		lineSprite->setRotation(-angle);

		fireStartCurveLines.push_back(lineSprite);
	}

	// �� ���ֵ�� �༭��
	for (int i = 0; i < fireStartCurvePointBtnVec.size(); ++i) {
		Vec2 point = fireStartCurvePointBtnVec.at(i)->getPosition();
		// �����ֵ�ı༭��
		auto editBox = ExEditBox::create(Size(35, 25), ui::Scale9Sprite::create("editBg.png"));
		editBox->setAnchorPoint(Vec2(0.5, 0));
		editBox->setPosition(Vec2(point.x, curvePanelSize.height + 5));
		editBox->setInputMode(ExEditBox::InputMode::NUMERIC);
		editBox->setFontColor(Color3B::BLACK);
		editBox->setFontSize(20);
		char name[50];
		sprintf(name, "fireStartCurveRandEditBox_%d", i);
		editBox->setName(name);

		char value[20];
		sprintf(value, "%.2f", nowEditingEmitterVarietyValue->curvePoints.at(i).z);
		editBox->setText(value);

		editBox->setDelegate(this);
		curvePanel->addChild(editBox);

		fireStartCurveRandEditBoxVec.push_back(editBox);
	}

	// �� ���� ���ֵ�༭�� ����
	for (int i = 0; i < fireStartCurvePointBtnVec.size(); ++i) {
		auto lineSprite = Sprite::create("coord_xu_line.png");
		curvePanel->addChild(lineSprite);


		Vec2 point = fireStartCurvePointBtnVec.at(i)->getPosition();
		float angle = 90;
		float dis = curvePanelSize.height - point.y;

		Vec2 realPos = Vec2(point.x, point.y + dis / 2);

		lineSprite->setScaleX(dis / lineSprite->getContentSize().width);
		lineSprite->setScaleY(2 / lineSprite->getContentSize().height);
		lineSprite->setColor(Color3B(255, 80, 0));
		lineSprite->setPosition(realPos);
		lineSprite->setRotation(-angle);

		fireStartCurveRandLines.push_back(lineSprite);
	}


	this->refreshFireStartCurvePoints(fireStartCurvePointMaxHeight);
}

void ParticleUiView::deleteFireStartCurvePoints(ui::Button* btn) {
	// ɾ��һ���� ��ť
	auto itor = fireStartCurvePointBtnVec.begin();
	int index = 0;
	while (itor != fireStartCurvePointBtnVec.end()) {
		if ((*itor) == btn && index != 0 && index != fireStartCurvePointBtnVec.size() - 1) {
			(*itor)->removeFromParent();
			fireStartCurvePointBtnVec.erase(itor);

			// ɾ��һ����
			fireStartCurveLines.at(fireStartCurveLines.size()-1)->removeFromParent();
			fireStartCurveLines.pop_back();

			// ɾ����Ӧ����
			nowEditingEmitterVarietyValue->curvePoints.erase(nowEditingEmitterVarietyValue->curvePoints.begin() + index);

			// ɾ����Ӧ ���ֵ �༭��
			fireStartCurveRandEditBoxVec.at(index)->removeFromParent();
			fireStartCurveRandEditBoxVec.erase(fireStartCurveRandEditBoxVec.begin() + index);
			// �ٰѺ���� ���ֵ �༭������ָ���
			for (int i = index; i < fireStartCurveRandEditBoxVec.size(); ++i){
				char name[50];
				sprintf(name, "fireStartCurveRandEditBox_%d", i);
				fireStartCurveRandEditBoxVec.at(i)->setName(name);
			}

			// ɾ����Ӧ ���� ���ֵ�༭�� ����
			fireStartCurveRandLines.at(index)->removeFromParent();
			fireStartCurveRandLines.erase(fireStartCurveRandLines.begin() + index);

			break;
		}
		index++;
		itor++;
	}
	

	this->refreshFireStartCurvePoints(fireStartCurvePointMaxHeight);
}

void ParticleUiView::refreshFireStartCurvePoints(float maxHeight) {
	auto curvePanel = seekByName(singleRootNode, "curveMap_fireStart");
	Size curvePanelSize = seekByName(singleRootNode, "curveMap_fireStart")->getContentSize();
	Vec2 curvePanelPos = Vec2(curvePanelSize.width / 2, curvePanelSize.height / 2);

	
	// ���ȸ�������
	if (fireStartCurvePointBtnVec.size() == nowEditingEmitterVarietyValue->curvePoints.size()) {
		for (int i = 0; i < fireStartCurvePointBtnVec.size(); ++i) {
			Vec2 pos = fireStartCurvePointBtnVec.at(i)->getPosition();
			float height = (pos.y - curvePanelSize.height / 2) / (curvePanelSize.height / 2) * fireStartCurvePointMaxHeight;
			if (fabsf(height) > maxHeight) {
				height = height / fabsf(height) * maxHeight;
			}
			nowEditingEmitterVarietyValue->curvePoints.at(i) = Vec3(pos.x / curvePanelSize.width, height, nowEditingEmitterVarietyValue->curvePoints.at(i).z );
		}

	}
	nowEditingEmitterVarietyValue->isSetCurveKB = false;
	

	// ���� ���� �� �㰴ť
	
	auto itor = fireStartCurvePointBtnVec.begin();
	int index = 0;
	while (itor != fireStartCurvePointBtnVec.end()) {
		Vec2 inPanelPos = Vec2(nowEditingEmitterVarietyValue->curvePoints.at(index).x * curvePanelSize.width, curvePanelPos.y + (nowEditingEmitterVarietyValue->curvePoints.at(index).y / maxHeight) * curvePanelSize.height / 2);
		(*itor)->setPosition(inPanelPos);

		index++;
		itor++;
	}

	

	// ���� ���ӵ� �� ��
	for (int i = 0; i < fireStartCurveLines.size(); ++i) {
		Vec2 point1 = fireStartCurvePointBtnVec.at(i)->getPosition();
		Vec2 point2 = fireStartCurvePointBtnVec.at(i + 1)->getPosition();
		Vec2 realPos = Vec2(point1.x + (point2.x - point1.x) / 2, point1.y + (point2.y - point1.y) / 2);

		float angle = Vec2(point1.x - point2.x, point1.y - point2.y).getAngle() / P_PI * 180;
		float dis = point1.getDistance(point2);

		fireStartCurveLines.at(i)->setScaleX(dis / fireStartCurveLines.at(i)->getContentSize().width);
		fireStartCurveLines.at(i)->setPosition(realPos);
		fireStartCurveLines.at(i)->setRotation(-angle);

	}

	// ���� ����� ��ť
	for (int i = 0; i < fireStartCurveRandEditBoxVec.size(); ++i) {
		Vec2 point = fireStartCurvePointBtnVec.at(i)->getPosition();
		fireStartCurveRandEditBoxVec.at(i)->setPositionX(point.x);
	}

	// ���� ���� ����� ��ť ����
	for (int i = 0; i < fireStartCurveRandLines.size(); ++i) {
		Vec2 point = fireStartCurvePointBtnVec.at(i)->getPosition();
		float dis = curvePanelSize.height - point.y;

		fireStartCurveRandLines.at(i)->setPosition(Vec2(point.x , point.y + dis/2));
		fireStartCurveRandLines.at(i)->setScaleX(dis / fireStartCurveRandLines.at(i)->getContentSize().width);
	}

	
	fireStartCurvePointMaxHeight = maxHeight;
}

/////// ***************************** �����������������Ե� �������� ��� *****************************
void ParticleUiView::showParLifeCurvePoints() {
	isOpenParLifeCurvePanel = true;
	// �������
	auto curvePanel = seekByName(singleRootNode, "curveMap_parLife");
	curvePanel->setSwallowTouches(true);
	Size curvePanelSize = curvePanel->getContentSize();
	Vec2 curvePanelPos = Vec2(curvePanelSize.width / 2, curvePanelSize.height / 2);

	// ���� ���� ��ѡ�� �Ƿ�ѡ�У�����������ݴ��ڵ���2 �������ã�������ͣ��
	auto curvePoints = nowEditingEmitterVarietyValue->curvePoints;

	if (curvePoints.size() >= 2) {
		((CheckBox*)seekByName(singleRootNode, "isOpenParLifeProCheckBox"))->setSelectedState(true);
		nowEditingEmitterVarietyValue->pType = emitterPropertyType::curve;
	}
	else {
		((CheckBox*)seekByName(singleRootNode, "isOpenParLifeProCheckBox"))->setSelectedState(false);
		nowEditingEmitterVarietyValue->pType = emitterPropertyType::none;
		return;
	}

	// �ҵ����߶ȣ�����ߵ�ֵ�����ұߵ�ֵ
	parLifeCurvePointMaxHeight = 0;
	int leftPrecent = 1000;
	parLifeCurvePointLeftPrecent = 0;
	int rightPrecent = 0;
	parLifeCurvePointRightPrecent = 0;
	auto itor = curvePoints.begin();
	while (itor != curvePoints.end()) {
		if (fabsf((*itor).y) > parLifeCurvePointMaxHeight) {
			parLifeCurvePointMaxHeight = (*itor).y * ((*itor).y / fabsf((*itor).y));
		}
		if ((*itor).x < leftPrecent) {
			leftPrecent = (*itor).x;
		}
		if ((*itor).x > rightPrecent) {
			rightPrecent = (*itor).x;
		}

		itor++;
	}
	parLifeCurvePointLeftPrecent = leftPrecent;
	parLifeCurvePointRightPrecent = rightPrecent;
	
	//// �������֣��༭��...
	char maxHeightStr[20];
	char minHeightStr[20];
	char leftPrecentStr[20];
	char rightPrecentStr[20];
	sprintf(maxHeightStr, "%.2f", parLifeCurvePointMaxHeight);
	sprintf(minHeightStr, "-%.2f", parLifeCurvePointMaxHeight);
	sprintf(leftPrecentStr, "%.2f", parLifeCurvePointLeftPrecent);
	sprintf(rightPrecentStr, "%.2f", parLifeCurvePointRightPrecent);
	((ExEditBox*)seekByName(singleRootNode, "topYInput_parLife"))->setText(maxHeightStr);
	((Text*)seekByName(singleRootNode, "downYWord_parLife"))->setString(minHeightStr);
	((ExEditBox*)seekByName(singleRootNode, "leftXInput_parLife"))->setText(leftPrecentStr);
	((ExEditBox*)seekByName(singleRootNode, "rightXInput_parLife"))->setText(rightPrecentStr);

	// �ӵ�
	itor = curvePoints.begin();
	while (itor != curvePoints.end()) {
		float xPos = ((*itor).x - parLifeCurvePointLeftPrecent) / (parLifeCurvePointRightPrecent - parLifeCurvePointLeftPrecent);
		Vec2 inPanelPos = Vec2(curvePanelPos.x - curvePanelSize.width / 2 + xPos * curvePanelSize.width, curvePanelPos.y + ((*itor).y / parLifeCurvePointMaxHeight) * curvePanelSize.height / 2);

		// ��һ�� ����������еĵ� ��ť
		auto curvePoint = ui::Button::create(
			"coord_point1.png",
			"coord_point2.png");

		curvePoint->setPosition(inPanelPos);
		parLifeCurvePointBtnVec.push_back(curvePoint);
		curvePoint->setSwallowTouches(false);
		curvePanel->addChild(curvePoint);
		curvePoint->addTouchEventListener(CC_CALLBACK_2(ParticleUiView::parLifeCurvePointCall, this));

		itor++;
	}

	// ���� �����ӵ���� �� 
	for (int i = 0; i < parLifeCurvePointBtnVec.size() - 1; ++i) {
		auto lineSprite = Sprite::create("coord_line.png");
		curvePanel->addChild(lineSprite);

		Vec2 point1 = parLifeCurvePointBtnVec.at(i)->getPosition();
		Vec2 point2 = parLifeCurvePointBtnVec.at(i + 1)->getPosition();
		Vec2 realPos = Vec2(point1.x + (point2.x - point1.x) / 2, point1.y + (point2.y - point1.y) / 2);

		float angle = Vec2(point1.x - point2.x, point1.y - point2.y).getAngle() / P_PI * 180;
		float dis = point1.getDistance(point2);

		lineSprite->setScaleX(dis / lineSprite->getContentSize().width);
		lineSprite->setScaleY(1 / lineSprite->getContentSize().height);
		lineSprite->setPosition(realPos);
		lineSprite->setRotation(-angle);

		parLifeCurveLines.push_back(lineSprite);

	}
	// �� ���ֵ�� �༭��
	for (int i = 0; i < parLifeCurvePointBtnVec.size(); ++i) {
		Vec2 point = parLifeCurvePointBtnVec.at(i)->getPosition();
		// �����ֵ�ı༭��
		auto editBox = ExEditBox::create(Size(35, 25), ui::Scale9Sprite::create("editBg.png"));
		editBox->setAnchorPoint(Vec2(0.5, 0));
		editBox->setPosition(Vec2(point.x, curvePanelSize.height + 5));
		editBox->setInputMode(ExEditBox::InputMode::NUMERIC);
		editBox->setFontColor(Color3B::BLACK);
		editBox->setFontSize(20);
		char name[50];
		sprintf(name, "parLifeCurveRandEditBox_%d", i);
		editBox->setName(name);
		char value[20];
		sprintf(value, "%.2f", curvePoints.at(i).z);
		editBox->setText(value);
		editBox->setDelegate(this);
		curvePanel->addChild(editBox);

		parLifeCurveRandEditBoxVec.push_back(editBox);
	}
	// �� ���� ���ֵ�༭�� ����
	for (int i = 0; i < parLifeCurvePointBtnVec.size(); ++i) {
		auto lineSprite = Sprite::create("coord_xu_line.png");
		curvePanel->addChild(lineSprite);


		Vec2 point = parLifeCurvePointBtnVec.at(i)->getPosition();
		float angle = 90;
		float dis = curvePanelSize.height - point.y;

		Vec2 realPos = Vec2(point.x, point.y + dis / 2);

		lineSprite->setScaleX(dis / lineSprite->getContentSize().width);
		lineSprite->setScaleY(2 / lineSprite->getContentSize().height);
		lineSprite->setColor(Color3B(255, 80, 0));
		lineSprite->setPosition(realPos);
		lineSprite->setRotation(-angle);

		parLifeCurveRandLines.push_back(lineSprite);
	}

}

void ParticleUiView::hideParLifeCurvePoints() {
	isOpenParLifeCurvePanel = false;
	// ɾ�㰴ť
	auto itor = parLifeCurvePointBtnVec.begin();
	while (itor != parLifeCurvePointBtnVec.end()) {
		(*itor)->removeFromParent();

		itor++;
	}
	parLifeCurvePointBtnVec.clear();

	// ɾ ���ӵ����
	auto itor2 = parLifeCurveLines.begin();
	while (itor2 != parLifeCurveLines.end()) {
		(*itor2)->removeFromParent();

		itor2++;
	}
	parLifeCurveLines.clear();

	// ɾ���ֵ�ı༭��
	auto itor3 = parLifeCurveRandEditBoxVec.begin();
	while (itor3 != parLifeCurveRandEditBoxVec.end()) {
		(*itor3)->removeFromParent();

		itor3++;
	}
	parLifeCurveRandEditBoxVec.clear();

	// ɾ���� ���ֵ�༭��� ��
	auto itor4 = parLifeCurveRandLines.begin();
	while (itor4 != parLifeCurveRandLines.end()) {
		(*itor4)->removeFromParent();

		itor4++;
	}
	parLifeCurveRandLines.clear();
}

void ParticleUiView::addParLifeCurvePoints(Vec2 pos) {
	auto curvePanel = seekByName(singleRootNode, "curveMap_parLife");
	Size curvePanelSize = curvePanel->getContentSize();
	Vec2 curvePanelPos = curvePanel->convertToWorldSpace(Vec2::ZERO);


	Vec2 inPanelPos = Vec2(pos.x - curvePanelPos.x, pos.y - curvePanelPos.y);

	Vec2 value = Vec2( (inPanelPos.x / curvePanelSize.width) * (parLifeCurvePointRightPrecent - parLifeCurvePointLeftPrecent) + parLifeCurvePointLeftPrecent , (inPanelPos.y - curvePanelSize.height / 2) / (curvePanelSize.height / 2) * parLifeCurvePointMaxHeight);

	// �ҵ���ȷ��λ�ò���
	int size = parLifeCurvePointBtnVec.size();
	for (int i = 1; i < size; ++i) {
		if (inPanelPos.x <= parLifeCurvePointBtnVec.at(i)->getPositionX()) {
			auto curvePoint = ui::Button::create(
				"coord_point1.png",
				"coord_point2.png");

			curvePoint->setPosition(inPanelPos);
			curvePoint->setSwallowTouches(false);
			curvePanel->addChild(curvePoint);
			curvePoint->addTouchEventListener(CC_CALLBACK_2(ParticleUiView::parLifeCurvePointCall, this));

			parLifeCurvePointBtnVec.insert(parLifeCurvePointBtnVec.begin() + i, curvePoint);
			nowEditingEmitterVarietyValue->curvePoints.insert(nowEditingEmitterVarietyValue->curvePoints.begin() + i, Vec3(value.x, value.y, 0));
			break;
		}

	}
	auto lineItor = parLifeCurveLines.begin();
	while (lineItor != parLifeCurveLines.end()) {
		(*lineItor)->removeFromParent();
		lineItor++;
	}
	parLifeCurveLines.clear();
	// ɾ���ֵ�ı༭��
	auto randEditBoxItor = parLifeCurveRandEditBoxVec.begin();
	while (randEditBoxItor != parLifeCurveRandEditBoxVec.end()) {
		(*randEditBoxItor)->removeFromParent();
		randEditBoxItor++;
	}
	parLifeCurveRandEditBoxVec.clear();

	// ɾ���� ���ֵ�༭��� ��
	auto randLineItor = parLifeCurveRandLines.begin();
	while (randLineItor != parLifeCurveRandLines.end()) {
		(*randLineItor)->removeFromParent();
		randLineItor++;
	}
	parLifeCurveRandLines.clear();

	// ����
	for (int i = 0; i < parLifeCurvePointBtnVec.size() - 1; ++i) {
		auto lineSprite = Sprite::create("coord_line.png");
		curvePanel->addChild(lineSprite);

		Vec2 point1 = parLifeCurvePointBtnVec.at(i)->getPosition();
		Vec2 point2 = parLifeCurvePointBtnVec.at(i + 1)->getPosition();
		Vec2 realPos = Vec2(point1.x + (point2.x - point1.x) / 2, point1.y + (point2.y - point1.y) / 2);

		float angle = Vec2(point1.x - point2.x, point1.y - point2.y).getAngle() / P_PI * 180;
		float dis = point1.getDistance(point2);

		lineSprite->setScaleX(dis / lineSprite->getContentSize().width);
		lineSprite->setScaleY(1 / lineSprite->getContentSize().height);
		lineSprite->setPosition(realPos);
		lineSprite->setRotation(-angle);

		parLifeCurveLines.push_back(lineSprite);
	}

	// �� ���ֵ�� �༭��
	for (int i = 0; i < parLifeCurvePointBtnVec.size(); ++i) {
		Vec2 point = parLifeCurvePointBtnVec.at(i)->getPosition();
		// �����ֵ�ı༭��
		auto editBox = ExEditBox::create(Size(35, 25), ui::Scale9Sprite::create("editBg.png"));
		editBox->setAnchorPoint(Vec2(0.5, 0));
		editBox->setPosition(Vec2(point.x, curvePanelSize.height + 5));
		editBox->setInputMode(ExEditBox::InputMode::NUMERIC);
		editBox->setFontColor(Color3B::BLACK);
		editBox->setFontSize(20);
		char name[50];
		sprintf(name, "parLifeCurveRandEditBox_%d", i);
		editBox->setName(name);

		char value[20];
		sprintf(value, "%.2f", nowEditingEmitterVarietyValue->curvePoints.at(i).z);
		editBox->setText(value);

		editBox->setDelegate(this);
		curvePanel->addChild(editBox);

		parLifeCurveRandEditBoxVec.push_back(editBox);
	}

	// �� ���� ���ֵ�༭�� ����
	for (int i = 0; i < parLifeCurvePointBtnVec.size(); ++i) {
		auto lineSprite = Sprite::create("coord_xu_line.png");
		curvePanel->addChild(lineSprite);


		Vec2 point = parLifeCurvePointBtnVec.at(i)->getPosition();
		float angle = 90;
		float dis = curvePanelSize.height - point.y;

		Vec2 realPos = Vec2(point.x, point.y + dis / 2);

		lineSprite->setScaleX(dis / lineSprite->getContentSize().width);
		lineSprite->setScaleY(2 / lineSprite->getContentSize().height);
		lineSprite->setColor(Color3B(255, 80, 0));
		lineSprite->setPosition(realPos);
		lineSprite->setRotation(-angle);

		parLifeCurveRandLines.push_back(lineSprite);
	}


	this->refreshParLifeCurvePoints(parLifeCurvePointMaxHeight);
}

void ParticleUiView::deleteParLifeCurvePoints(ui::Button* btn) {
	// ɾ��һ���� ��ť
	auto itor = parLifeCurvePointBtnVec.begin();
	int index = 0;
	while (itor != parLifeCurvePointBtnVec.end()) {
		if ((*itor) == btn && index != 0 && index != parLifeCurvePointBtnVec.size() - 1) {
			(*itor)->removeFromParent();
			parLifeCurvePointBtnVec.erase(itor);

			// ɾ��һ����
			parLifeCurveLines.at(parLifeCurveLines.size() - 1)->removeFromParent();
			parLifeCurveLines.pop_back();

			// ɾ����Ӧ����
			nowEditingEmitterVarietyValue->curvePoints.erase(nowEditingEmitterVarietyValue->curvePoints.begin() + index);

			// ɾ����Ӧ ���ֵ �༭��
			parLifeCurveRandEditBoxVec.at(index)->removeFromParent();
			parLifeCurveRandEditBoxVec.erase(parLifeCurveRandEditBoxVec.begin() + index);

			// �ٰѺ���� ���ֵ �༭������ָ���
			for (int i = index; i < parLifeCurveRandEditBoxVec.size(); ++i){
				char name[50];
				sprintf(name, "parLifeCurveRandEditBox_%d", i);
				parLifeCurveRandEditBoxVec.at(i)->setName(name);
			}

			// ɾ����Ӧ ���� ���ֵ�༭�� ����
			parLifeCurveRandLines.at(index)->removeFromParent();
			parLifeCurveRandLines.erase(parLifeCurveRandLines.begin() + index);

			break;
		}
		index++;
		itor++;
	}


	this->refreshParLifeCurvePoints(parLifeCurvePointMaxHeight);
}

void ParticleUiView::refreshParLifeCurvePoints(float maxHeight) {
	auto curvePanel = seekByName(singleRootNode, "curveMap_parLife");
	Size curvePanelSize = curvePanel->getContentSize();
	Vec2 curvePanelPos = Vec2(curvePanelSize.width / 2, curvePanelSize.height / 2);

	// ���ҵ��ϵ� ��������ı߽�
	float oldLeftPrecent = 10000;
	float oldRightPrecent = 0;
	auto curvePoints = nowEditingEmitterVarietyValue->curvePoints;
	{
		auto itor = curvePoints.begin();
		while (itor != curvePoints.end()) {
			if (fabsf((*itor).y) > parLifeCurvePointMaxHeight) {
				parLifeCurvePointMaxHeight = (*itor).y * ((*itor).y / fabsf((*itor).y));
			}
			if ((*itor).x < oldLeftPrecent) {
				oldLeftPrecent = (*itor).x;
			}
			if ((*itor).x > oldRightPrecent) {
				oldRightPrecent = (*itor).x;
			}
			itor++;
		}
	}

	// ���ȸ�������
	if (parLifeCurvePointBtnVec.size() == nowEditingEmitterVarietyValue->curvePoints.size()) {
		for (int i = 0; i < parLifeCurvePointBtnVec.size(); ++i) {
			Vec2 pos = parLifeCurvePointBtnVec.at(i)->getPosition();
			float height = (pos.y - curvePanelSize.height / 2) / (curvePanelSize.height / 2) * parLifeCurvePointMaxHeight;
			if (fabsf(height) > maxHeight) {
				height = height / fabsf(height) * maxHeight;
			}
			//float xPos = oldLeftPrecent + (pos.x / curvePanelSize.width) * (oldRightPrecent - oldLeftPrecent);
			float xPos = pos.x / curvePanelSize.width;
			xPos = parLifeCurvePointLeftPrecent + xPos * (parLifeCurvePointRightPrecent - parLifeCurvePointLeftPrecent);
			nowEditingEmitterVarietyValue->curvePoints.at(i) = Vec3( xPos , height, nowEditingEmitterVarietyValue->curvePoints.at(i).z);
		}

	}
	nowEditingEmitterVarietyValue->isSetCurveKB = false;

	// ���� ���� �� �㰴ť

	auto itor = parLifeCurvePointBtnVec.begin();
	int index = 0;
	while (itor != parLifeCurvePointBtnVec.end()) {
		float xPos = (nowEditingEmitterVarietyValue->curvePoints.at(index).x - parLifeCurvePointLeftPrecent) / (parLifeCurvePointRightPrecent - parLifeCurvePointLeftPrecent);
		Vec2 inPanelPos = Vec2(xPos * curvePanelSize.width, curvePanelPos.y + (nowEditingEmitterVarietyValue->curvePoints.at(index).y / maxHeight) * curvePanelSize.height / 2);
		(*itor)->setPosition(inPanelPos);

		index++;
		itor++;
	}



	// ���� ���ӵ� �� ��
	for (int i = 0; i < parLifeCurveLines.size(); ++i) {
		Vec2 point1 = parLifeCurvePointBtnVec.at(i)->getPosition();
		Vec2 point2 = parLifeCurvePointBtnVec.at(i + 1)->getPosition();
		Vec2 realPos = Vec2(point1.x + (point2.x - point1.x) / 2, point1.y + (point2.y - point1.y) / 2);

		float angle = Vec2(point1.x - point2.x, point1.y - point2.y).getAngle() / P_PI * 180;
		float dis = point1.getDistance(point2);

		parLifeCurveLines.at(i)->setScaleX(dis / parLifeCurveLines.at(i)->getContentSize().width);
		parLifeCurveLines.at(i)->setPosition(realPos);
		parLifeCurveLines.at(i)->setRotation(-angle);

	}

	// ���� ����� ��ť
	for (int i = 0; i < parLifeCurveRandEditBoxVec.size(); ++i) {
		Vec2 point = parLifeCurvePointBtnVec.at(i)->getPosition();
		parLifeCurveRandEditBoxVec.at(i)->setPositionX(point.x);
	}

	// ���� ���� ����� ��ť ����
	for (int i = 0; i < parLifeCurveRandLines.size(); ++i) {
		Vec2 point = parLifeCurvePointBtnVec.at(i)->getPosition();
		float dis = curvePanelSize.height - point.y;

		parLifeCurveRandLines.at(i)->setPosition(Vec2(point.x, point.y + dis / 2));
		parLifeCurveRandLines.at(i)->setScaleX(dis / parLifeCurveRandLines.at(i)->getContentSize().width);
	}


	parLifeCurvePointMaxHeight = maxHeight;
}

//////***************************** ����ʱ���� ����ɫ�� ���Ե� �������� ��� *****************************
void ParticleUiView::showFireStartCurveColors() {
	isOpenFireStartCurveColorPanel = true;

	auto colorLine = seekByName(singleRootNode, "curveColorValueShowLine");
	Size colorLineSize = colorLine->getContentSize();

	char durationStr[20];
	sprintf(durationStr,"%.2f",nowEditingSignalPar->_duration);
	((Text*)seekByName(singleRootNode, "curveColorValue_rightDuration"))->setString(durationStr);

	auto curveColors = nowEditingEmitterColorValue->curveColors;

	// ���� ��ɫ ��ť
	auto itor = curveColors.begin();
	while (itor != curveColors.end()) {

		Vec2 inPanelPos = Vec2( (*itor).x * colorLineSize.width , colorLineSize.height + 5); 

		auto curvePoint = ui::Button::create(
			"curveColorPointer.png",
			"curveColorPointer.png");
		curvePoint->setAnchorPoint(Vec2(0.5,0));
		curvePoint->setPosition(inPanelPos);
		fireStartCurveColorBtnVec.push_back(curvePoint);
		curvePoint->setSwallowTouches(false);
		fireStartCurveColorLine->addChild(curvePoint);

		curvePoint->setColor((*itor).colorY);

		curvePoint->addTouchEventListener(CC_CALLBACK_2(ParticleUiView::fireStartCurveColorCall, this));

		itor++;
	}

	this->refreshFireStartCurveColors();
}
void ParticleUiView::hideFireStartCurveColors() {
	isOpenFireStartCurveColorPanel = false;
	// ɾ�㰴ť
	auto itor = fireStartCurveColorBtnVec.begin();
	while (itor != fireStartCurveColorBtnVec.end()) {
		(*itor)->removeFromParent();
		itor++;
	}
	fireStartCurveColorBtnVec.clear();
}
void ParticleUiView::addFireStartCurveColors(float posX) {
	auto colorLine = seekByName(singleRootNode, "curveColorValueShowLine");
	Size colorLineSize = colorLine->getContentSize();
	Vec2 colorLinePos = colorLine->convertToWorldSpace(Vec2::ZERO);


	Vec2 inPanelPos = Vec2( (posX - colorLinePos.x) , colorLineSize.height + 5);

	auto curvePoint = ui::Button::create(
		"curveColorPointer.png",
		"curveColorPointer.png");
	curvePoint->setAnchorPoint(Vec2(0.5, 0));
	curvePoint->setPosition(inPanelPos);
	fireStartCurveColorBtnVec.push_back(curvePoint);
	curvePoint->setSwallowTouches(false);
	fireStartCurveColorLine->addChild(curvePoint);
	curvePoint->addTouchEventListener(CC_CALLBACK_2(ParticleUiView::fireStartCurveColorCall, this));

	colorCurvePoint colorPoint;
	colorPoint.x = inPanelPos.x / colorLineSize.width;
	colorPoint.colorY = Color3B(255, 255, 255);
	nowEditingEmitterColorValue->curveColors.push_back(colorPoint);

	this->refreshFireStartCurveColors();
}
void ParticleUiView::deleteFireStartCurveColors(ui::Button* btn) {
	if (fireStartCurveColorBtnVec.size() <= 1) {
		return;
	}

	if ((Widget*)btn == nowEditingColorNode) {
		this->hideSetColorPanel();
	}

	auto itor = fireStartCurveColorBtnVec.begin();
	int index = 0;
	while (itor != fireStartCurveColorBtnVec.end()) {
		if ((*itor) == btn) {
			(*itor)->removeFromParent();
			fireStartCurveColorBtnVec.erase(itor);

			// ɾ����Ӧ����
			nowEditingEmitterColorValue->curveColors.erase(nowEditingEmitterColorValue->curveColors.begin() + index);

			break;
		}
		index++;
		itor++;
	}

	this->refreshFireStartCurveColors();
}
void ParticleUiView::refreshFireStartCurveColors() {
	CCASSERT(fireStartCurveColorLine, "refreshFireStartCurveColors must have fireStartCurveColorLine !!");
	CCASSERT(nowEditingEmitterColorValue->curveColors.size() < 28, "refreshFireStartCurveColors :nowEditingEmitterColorValue num too many !!");
	auto colorLine = seekByName(singleRootNode, "curveColorValueShowLine");
	Vec2 colorLinePos = colorLine->convertToWorldSpace(Vec2::ZERO);
	Size colorLineSize = colorLine->getContentSize();

	// ��һ����
	int btnSize = fireStartCurveColorBtnVec.size();
	for (int i = 0; i < btnSize - 1; ++i){
		for (int j = i; j < btnSize; ++j){
			if (fireStartCurveColorBtnVec.at(i)->getPositionX() > fireStartCurveColorBtnVec.at(j)->getPositionX()){
				std::swap(fireStartCurveColorBtnVec.at(i), fireStartCurveColorBtnVec.at(j));
			}
		}
	}

	// �ȸ���һ������
	if (fireStartCurveColorBtnVec.size() == nowEditingEmitterColorValue->curveColors.size()){
		auto itor = fireStartCurveColorBtnVec.begin();
		int index = 0;
		while (itor != fireStartCurveColorBtnVec.end()) {
			nowEditingEmitterColorValue->curveColors.at(index).x = (*itor)->getPositionX() / colorLineSize.width;
			nowEditingEmitterColorValue->curveColors.at(index).colorY = (*itor)->getColor();

			index++;
			itor++;
		}
	}
	
	const int maxNum = 30; 
	static float pos[maxNum];
	static float colorR[maxNum];
	static float colorG[maxNum];
	static float colorB[maxNum];

	int index = 0;
	if (nowEditingEmitterColorValue->curveColors.size() > 0){
		
		int size = nowEditingEmitterColorValue->curveColors.size();

		pos[index] = 0;
		colorR[index] = nowEditingEmitterColorValue->curveColors.at(0).colorY.r;
		colorG[index] = nowEditingEmitterColorValue->curveColors.at(0).colorY.g;
		colorB[index] = nowEditingEmitterColorValue->curveColors.at(0).colorY.b;
		index++;

		auto itor = nowEditingEmitterColorValue->curveColors.begin();
		while (itor != nowEditingEmitterColorValue->curveColors.end()) {
			pos[index] = (*itor).x;
			colorR[index] = (*itor).colorY.r;
			colorG[index] = (*itor).colorY.g;
			colorB[index] = (*itor).colorY.b;

			index++;
			itor++;
		}

		
		pos[index] = 1;
		colorR[index] = nowEditingEmitterColorValue->curveColors.at(size-1).colorY.r;
		colorG[index] = nowEditingEmitterColorValue->curveColors.at(size-1).colorY.g;
		colorB[index] = nowEditingEmitterColorValue->curveColors.at(size-1).colorY.b;
		index++;
	}

	



	if (index >= 1) {
		fireStartCurveColorLine->setIntArg("colorNum", index);
		fireStartCurveColorLine->setFloatVecArg("pos", pos, index);  
		fireStartCurveColorLine->setFloatVecArg("colorR", colorR, index);
		fireStartCurveColorLine->setFloatVecArg("colorG", colorG, index);
		fireStartCurveColorLine->setFloatVecArg("colorB", colorB, index);
		
		
	}
}

//////***************************** �������������� ����ɫ�� ���Ե� �������� ��� *****************************
void ParticleUiView::showParLifeCurveColors() {
	isOpenParLifeCurveColorPanel = true;
	// �����
	auto panel = seekByName(singleRootNode, "parLifeProColorValuePanel"); 
	panel->setVisible(true);
	
	// ��ѡ�� �Ƿ�����
	auto curveColors = nowEditingEmitterColorValue->curveColors;
	if (curveColors.size() >= 2) {
		((CheckBox*)seekByName(singleRootNode, "isOpenParLifeProColorCheckBox"))->setSelectedState(true);
		nowEditingEmitterColorValue->pType = emitterPropertyType::curve;
	}
	else {
		((CheckBox*)seekByName(singleRootNode, "isOpenParLifeProColorCheckBox"))->setSelectedState(false);
		nowEditingEmitterColorValue->pType = emitterPropertyType::none;
		return;
	}

	// ������ɫ�� shader
	auto colorLine = seekByName(singleRootNode, "parLifeProColorValueShowLine");
	Size colorLineSize = colorLine->getContentSize();
	if (!parLifeCurveColorLine) {
		parLifeCurveColorLine = ShaderSprite::create("curveColorLineShader.png");
		parLifeCurveColorLine->setShaderFile("curveColorLine.vsh", "curveColorLine.fsh");
		parLifeCurveColorLine->setPosition(Vec2(colorLineSize.width*0.5, colorLineSize.height*0.5));

		colorLine->addChild(parLifeCurveColorLine);
	}

	// �����������ߵ�precent
	parLifeCurveColorLeftPrecent = curveColors.at(0).x;
	parLifeCurveColorRightPrecent = curveColors.at(curveColors.size() - 1).x;

	char leftPrecentStr[20];
	sprintf(leftPrecentStr, "%.2f", parLifeCurveColorLeftPrecent);
	((ExEditBox*)seekByName(singleRootNode, "parLifeProColor_leftSideInput"))->setText(leftPrecentStr);

	char rightPrecentStr[20];
	sprintf(rightPrecentStr, "%.2f", parLifeCurveColorRightPrecent);
	((ExEditBox*)seekByName(singleRootNode, "parLifeProColor_rightSideInput"))->setText(rightPrecentStr);

	
	// ���� ��ɫ ��ť
	auto itor = curveColors.begin();
	while (itor != curveColors.end()) {
		float posX = ((*itor).x - parLifeCurveColorLeftPrecent) / (parLifeCurveColorRightPrecent - parLifeCurveColorLeftPrecent) * colorLineSize.width;
		Vec2 inPanelPos = Vec2(posX, colorLineSize.height + 5);

		auto curvePoint = ui::Button::create(
			"curveColorPointer.png",
			"curveColorPointer.png");
		curvePoint->setAnchorPoint(Vec2(0.5, 0));
		curvePoint->setPosition(inPanelPos);
		parLifeCurveColorBtnVec.push_back(curvePoint);
		curvePoint->setSwallowTouches(false);
		parLifeCurveColorLine->addChild(curvePoint);

		curvePoint->setColor((*itor).colorY);

		curvePoint->addTouchEventListener(CC_CALLBACK_2(ParticleUiView::parLifeCurveColorCall, this));

		itor++;
	}

	this->refreshParLifeCurveColors();
}

void ParticleUiView::hideParLifeCurveColors() {
	// �ر����
	auto panel = seekByName(singleRootNode, "parLifeProColorValuePanel");
	panel->setVisible(false);

	isOpenParLifeCurveColorPanel = false;

	this->hideSetColorPanel();

	// ɾ�㰴ť
	auto itor = parLifeCurveColorBtnVec.begin();
	while (itor != parLifeCurveColorBtnVec.end()) {
		(*itor)->removeFromParent();
		itor++;
	}
	parLifeCurveColorBtnVec.clear();

	if (parLifeCurveColorLine) {
		if (parLifeCurveColorLine->getParent()) {
			parLifeCurveColorLine->removeFromParent();
		}
		parLifeCurveColorLine = nullptr;
	}

}
void ParticleUiView::addParLifeCurveColors(float posX) {
	auto colorLine = seekByName(singleRootNode, "parLifeProColorValueShowLine");
	Size colorLineSize = colorLine->getContentSize();
	Vec2 colorLinePos = colorLine->convertToWorldSpace(Vec2::ZERO);

	Vec2 inPanelPos = Vec2((posX - colorLinePos.x), colorLineSize.height + 5);

	int index = 0;
	auto itor = parLifeCurveColorBtnVec.begin();
	while (itor != parLifeCurveColorBtnVec.end()) {
		if (inPanelPos.x < (*itor)->getPositionX()) {
			break;
		}
		index++;
		itor++;
	}
	auto curvePoint = ui::Button::create(
		"curveColorPointer.png",
		"curveColorPointer.png");
	curvePoint->setAnchorPoint(Vec2(0.5, 0));
	curvePoint->setPosition(inPanelPos);
	parLifeCurveColorBtnVec.insert(parLifeCurveColorBtnVec.begin() + index , curvePoint);
	curvePoint->setSwallowTouches(false);
	parLifeCurveColorLine->addChild(curvePoint);
	curvePoint->addTouchEventListener(CC_CALLBACK_2(ParticleUiView::parLifeCurveColorCall, this));

	colorCurvePoint colorPoint;
	colorPoint.x = parLifeCurveColorLeftPrecent + inPanelPos.x / colorLineSize.width * (parLifeCurveColorRightPrecent - parLifeCurveColorLeftPrecent);
	colorPoint.colorY = Color3B(255, 255, 255);
	nowEditingEmitterColorValue->curveColors.insert(nowEditingEmitterColorValue->curveColors.begin() + index , colorPoint);

	this->refreshParLifeCurveColors();
}
void ParticleUiView::deleteParLifeCurveColors(ui::Button* btn) {
	if (parLifeCurveColorBtnVec.size() <= 2) {
		return;
	}

	// ����ɾ������������
	if (btn == parLifeCurveColorBtnVec.at(0) || btn == parLifeCurveColorBtnVec.at(parLifeCurveColorBtnVec.size() - 1)) {
		return;
	}

	if ((Widget*)btn == nowEditingColorNode) {
		this->hideSetColorPanel();
	}


	auto itor = parLifeCurveColorBtnVec.begin();
	int index = 0;
	while (itor != parLifeCurveColorBtnVec.end()) {
		if ((*itor) == btn) {
			(*itor)->removeFromParent();
			parLifeCurveColorBtnVec.erase(itor);

			// ɾ����Ӧ����
			nowEditingEmitterColorValue->curveColors.erase(nowEditingEmitterColorValue->curveColors.begin() + index);

			break;
		}
		index++;
		itor++;
	}

	this->refreshParLifeCurveColors();
}
void ParticleUiView::refreshParLifeCurveColors() {
	CCASSERT(parLifeCurveColorLine, "refreshParLifeCurveColors must have parLifeCurveColorLine !!");
	CCASSERT(nowEditingEmitterColorValue->curveColors.size() < 28, "refreshParLifeCurveColors :nowEditingEmitterColorValue num too many !!");
	auto colorLine = seekByName(singleRootNode, "parLifeProColorValueShowLine");
	Vec2 colorLinePos = colorLine->convertToWorldSpace(Vec2::ZERO);
	Size colorLineSize = colorLine->getContentSize();

	// ��һ����
	int btnSize = parLifeCurveColorBtnVec.size();
	for (int i = 0; i < btnSize - 1; ++i) {
		for (int j = i; j < btnSize; ++j) {
			if (parLifeCurveColorBtnVec.at(i)->getPositionX() > parLifeCurveColorBtnVec.at(j)->getPositionX()) {
				std::swap(parLifeCurveColorBtnVec.at(i), parLifeCurveColorBtnVec.at(j));
			}
		}
	}

	// �ȸ���һ������
	if (parLifeCurveColorBtnVec.size() == nowEditingEmitterColorValue->curveColors.size()) {
		auto itor = parLifeCurveColorBtnVec.begin();
		int index = 0;
		while (itor != parLifeCurveColorBtnVec.end()) {
			nowEditingEmitterColorValue->curveColors.at(index).x = parLifeCurveColorLeftPrecent + (*itor)->getPositionX() / colorLineSize.width * (parLifeCurveColorRightPrecent - parLifeCurveColorLeftPrecent);
			nowEditingEmitterColorValue->curveColors.at(index).colorY = (*itor)->getColor();

			index++;
			itor++;
		}
	}

	const int maxNum = 30;
	static float pos[maxNum];
	static float colorR[maxNum];
	static float colorG[maxNum];
	static float colorB[maxNum];

	int index = 0;
	if (nowEditingEmitterColorValue->curveColors.size() > 0) {

		int size = nowEditingEmitterColorValue->curveColors.size();

		auto itor = nowEditingEmitterColorValue->curveColors.begin();
		while (itor != nowEditingEmitterColorValue->curveColors.end()) {
			pos[index] = ((*itor).x - parLifeCurveColorLeftPrecent) / (parLifeCurveColorRightPrecent - parLifeCurveColorLeftPrecent);
			colorR[index] = (*itor).colorY.r;
			colorG[index] = (*itor).colorY.g;
			colorB[index] = (*itor).colorY.b;

			index++;
			itor++;
		}

	}

	if (index >= 1) {
		parLifeCurveColorLine->setIntArg("colorNum", index);
		parLifeCurveColorLine->setFloatVecArg("pos", pos, index);
		parLifeCurveColorLine->setFloatVecArg("colorR", colorR, index);
		parLifeCurveColorLine->setFloatVecArg("colorG", colorG, index);
		parLifeCurveColorLine->setFloatVecArg("colorB", colorB, index);

	}
}


///////////////////******* �� �������������ڵ��������
void ParticleUiView::showParLifeProValuePanel(singleProType proType) {
	nowSelectedSingleProType = proType;
	seekByName(singleRootNode, "parLifeProValuePanel")->setVisible(true); 

	CCDictionary* namesXML = CCDictionary::createWithContentsOfFile("singleProTypeStrings.xml");
	int index = (int)proType;
	char key[20];
	sprintf(key, "name%d", index);
	auto nameKey = namesXML->valueForKey(key);
	std::string name = nameKey->getCString();

	((Text*)seekByName(singleRootNode, "titleWord_parLife"))->setString(name);

	
	if (nowSelectedSingleProType == singleProType::speed_parLife ) {
		nowEditingEmitterVarietyValue = &nowEditingSignalPar->_moveSpeedOfLife;
	}
	else if (nowSelectedSingleProType == singleProType::acc_parLife) {
		nowEditingEmitterVarietyValue = &nowEditingSignalPar->_moveAccOfLife ;
	}
	else if (nowSelectedSingleProType == singleProType::moveAngle_parLife) {
		nowEditingEmitterVarietyValue = &nowEditingSignalPar->_moveAngleOfLife;
	}
	else if (nowSelectedSingleProType == singleProType::moveAngleSpeed_parLife) {
		nowEditingEmitterVarietyValue = &nowEditingSignalPar->_moveAngleSpeedOfLife;
	}
	else if (nowSelectedSingleProType == singleProType::gravityX_parLife) {
		nowEditingEmitterVarietyValue = &nowEditingSignalPar->_gravityXOfLife;
	}
	else if (nowSelectedSingleProType == singleProType::gravityY_parLife) {
		nowEditingEmitterVarietyValue = &nowEditingSignalPar->_gravityYOfLife;
	}
	else if (nowSelectedSingleProType == singleProType::size_parLife) {
		nowEditingEmitterVarietyValue = &nowEditingSignalPar->_sizeOfLife;
	}
	else if (nowSelectedSingleProType == singleProType::rotation_parLife) {
		nowEditingEmitterVarietyValue = &nowEditingSignalPar->_rotationOfLife;
	}
	else if (nowSelectedSingleProType == singleProType::rotationSpeed_parLife) {
		nowEditingEmitterVarietyValue = &nowEditingSignalPar->_rotationSpeedOfLife;
	}
	else if (nowSelectedSingleProType == singleProType::skewX_parLife) {
		nowEditingEmitterVarietyValue = &nowEditingSignalPar->_skewXOfLife;
	}
	else if (nowSelectedSingleProType == singleProType::skewXSpeed_parLife) {
		nowEditingEmitterVarietyValue = &nowEditingSignalPar->_skewXSpeedOfLife;
	}
	else if (nowSelectedSingleProType == singleProType::skewY_parLife) {
		nowEditingEmitterVarietyValue = &nowEditingSignalPar->_skewYOfLife;
	}
	else if (nowSelectedSingleProType == singleProType::skewYSpeed_parLife) {
		nowEditingEmitterVarietyValue = &nowEditingSignalPar->_skewYSpeedOfLife;
	}
	else if (nowSelectedSingleProType == singleProType::alpha_parLife) {
		nowEditingEmitterVarietyValue = &nowEditingSignalPar->_alphaOfLife;
	}
	
	else if (nowSelectedSingleProType == singleProType::color_parLife) {
		nowEditingEmitterColorValue = &nowEditingSignalPar->_colorOfLife;
		seekByName(singleRootNode, "parLifeProValuePanel")->setVisible(false);
		this->showParLifeCurveColors();
		return;
	}

	// ��ʾ���
	this->hideParLifeCurvePoints();
	this->showParLifeCurvePoints();

}
void ParticleUiView::hideParLifeProValuePanel() {
	this->hideParLifeCurvePoints();
	this->hideParLifeCurveColors();

	nowSelectedSingleProType = singleProType::singleProTypeNone;
	seekByName(singleRootNode, "parLifeProValuePanel")->setVisible(false);

	seekByName(singleRootNode, "parLifeBtn_color")->setColor(Color3B(255, 255, 255));

	if (nowEditingEmitterVarietyValue) {
		
		nowEditingEmitterVarietyValue = nullptr;
	}
}

// ����һ����������ֵ���ж�ֵ�Ƿ���ڻ�������������С
void ParticleUiView::setSliderValueWithMaxMin(ExSlider& slider, float value) {
	if (value > slider.getMaximumValue()) {
		//slider.setMaximumValue(value);
		slider.setMaximumValueWithoutSetValue(value);
		
	}
	else if (value < slider.getMinimumValue()) {
		//slider.setMinimumValue(value);
		slider.setMinimumValueWithoutSetValue(value);
	}
	slider.setValue(value);
	//slider.setValueWithoutChangedEvent(value);
}

void ParticleUiView::setSliderValueWithMaxMinNoSetValueEvent(ExSlider& slider, float value) {
	if(value > slider.getMaximumValue()) {
		slider.setMaximumValueWithoutSetValue(value);
	}
	else if (value < slider.getMinimumValue()) {
		slider.setMinimumValueWithoutSetValue(value);
	}
	slider.setValueWithoutChangedEvent(value);
}

void ParticleUiView::initSingleParUi() {
	std::string texName = nowEditingSignalPar->_texName;
	float subStart = texName.find("/");
	std::string realTexName = texName.substr(subStart+1,-1);

	textureList->setSelectedIndexByString(realTexName);
	{
		char textTem[20];
		sprintf(textTem, "%.2f", nowEditingSignalPar->_texAnchorPoint.x);
		((ExEditBox*)seekByName(singleRootNode, "textureAnchorPointX_Input"))->setText(textTem);
	}
	{
		char textTem[20];
		sprintf(textTem, "%.2f", nowEditingSignalPar->_texAnchorPoint.y);
		((ExEditBox*)seekByName(singleRootNode, "textureAnchorPointY_Input"))->setText(textTem);
	}

	setSliderValueWithMaxMinNoSetValueEvent(*((ExSlider*)seekByName(singleRootNode, "maxParNum_Slider")) , nowEditingSignalPar->_maxParticleNum);
	setSliderValueWithMaxMinNoSetValueEvent(*((ExSlider*)seekByName(singleRootNode, "duration_Slider")), nowEditingSignalPar->_duration);
	setSliderValueWithMaxMinNoSetValueEvent(*((ExSlider*)seekByName(singleRootNode, "fireRate_Slider")), nowEditingSignalPar->_emissionRate);
	((ExSlider*)seekByName(singleRootNode, "preFire_Slider"))->setMaximumValueWithoutSetValue(1);
	setSliderValueWithMaxMinNoSetValueEvent(*((ExSlider*)seekByName(singleRootNode, "preFire_Slider")), nowEditingSignalPar->_preFireDuration);
	((ExSlider*)seekByName(singleRootNode, "fireDelay_Slider"))->setMaximumValueWithoutSetValue(1);
	setSliderValueWithMaxMinNoSetValueEvent(*((ExSlider*)seekByName(singleRootNode, "fireDelay_Slider")), nowEditingSignalPar->_delayTime);

	// �༭��
	{
		char textTem[20];
		sprintf(textTem, "%d", nowEditingSignalPar->_maxParticleNum);
		((ExEditBox*)seekByName(singleRootNode, "maxParNum_Input"))->setText(textTem);
	}
	{
		char textTem[20];
		sprintf(textTem, "%.2f", nowEditingSignalPar->_duration);
		((ExEditBox*)seekByName(singleRootNode, "duration_Input"))->setText(textTem);
	}
	{
		char textTem[20];
		sprintf(textTem, "%.2f", nowEditingSignalPar->_emissionRate);
		((ExEditBox*)seekByName(singleRootNode, "fireRate_Input"))->setText(textTem);
	}
	{
		char textTem[20];
		sprintf(textTem, "%.2f", nowEditingSignalPar->_preFireDuration);
		((ExEditBox*)seekByName(singleRootNode, "preFire_Input"))->setText(textTem);
	}
	{
		char textTem[20];
		sprintf(textTem, "%.2f", nowEditingSignalPar->_delayTime);
		((ExEditBox*)seekByName(singleRootNode, "fireDelay_Input"))->setText(textTem);
	}


	// �Ƿ�ѭ��
	if (nowEditingSignalPar->_isLoop) {
		((CheckBox*)seekByName(singleRootNode, "isLoopCheckBox"))->setSelectedState(true);
	}
	else {
		((CheckBox*)seekByName(singleRootNode, "isLoopCheckBox"))->setSelectedState(false);
	}
	// �˶�ģʽ
	if (nowEditingSignalPar->_positionType == positionType::FREE ) {
		((CheckBox*)seekByName(singleRootNode, "moveMode_free"))->setSelectedState(true);
		((CheckBox*)seekByName(singleRootNode, "moveMode_relative"))->setSelectedState(false);
	}
	else if(nowEditingSignalPar->_positionType == positionType::RELATIVE){
		((CheckBox*)seekByName(singleRootNode, "moveMode_free"))->setSelectedState(false);
		((CheckBox*)seekByName(singleRootNode, "moveMode_relative"))->setSelectedState(true);
	}
	// �Ƕ�ģʽ
	if (nowEditingSignalPar->_angleType == fireAngleType::local) {
		((CheckBox*)seekByName(singleRootNode, "angleMode_global"))->setSelectedState(false);
		((CheckBox*)seekByName(singleRootNode, "angleMode_local"))->setSelectedState(true);
	}
	else if (nowEditingSignalPar->_angleType == fireAngleType::global) {
		((CheckBox*)seekByName(singleRootNode, "angleMode_global"))->setSelectedState(true);
		((CheckBox*)seekByName(singleRootNode, "angleMode_local"))->setSelectedState(false);
	}
	// ���ģʽ
	sourceBlend->setSelectedIndex(getIntByBlendType(nowEditingSignalPar->_sourceBlend));
	destBlend->setSelectedIndex(getIntByBlendType(nowEditingSignalPar->_destBlend));
	// ��������ģʽ
	this->setFireAreaMode(nowEditingSignalPar->_fireArea);
	

}

void ParticleUiView::setFireAreaMode(varietyFireAreaValue& areaData) {
	if (areaData.fAreaType == fireAreaType::rect) {
		fireAreaMode->setSelectedIndex(0);
		// ��Ӧ�Ĳ������򿪹ر�
		seekByName(singleRootNode, "rectModeLayer")->setVisible(true);
		seekByName(singleRootNode, "circleModeLayer")->setVisible(false);
		seekByName(singleRootNode, "polygonAndLineCommonLayer")->setVisible(false);
		seekByName(singleRootNode, "polygonModeLayer")->setVisible(false);

		((ExSlider*)seekByName(singleRootNode, "edgeFireWidth_Slider"))->setEnabled(false);

		((ExSlider*)seekByName(singleRootNode, "rectModeOutWidth_Slider"))->setEnabled(true);
		((ExSlider*)seekByName(singleRootNode, "rectModeInWidthSlider"))->setEnabled(true);
		((ExSlider*)seekByName(singleRootNode, "rectModeOutHeight_Slider"))->setEnabled(true);
		((ExSlider*)seekByName(singleRootNode, "rectModeInHeightSlider"))->setEnabled(true);
		// ���û�������ֵ�����ж������Сֵ��
		this->setSliderValueWithMaxMin(*((ExSlider*)seekByName(singleRootNode, "rectModeInWidthSlider")), areaData.inRect.width);
		this->setSliderValueWithMaxMin(*((ExSlider*)seekByName(singleRootNode, "rectModeOutWidth_Slider")), areaData.outRect.width);
		this->setSliderValueWithMaxMin(*((ExSlider*)seekByName(singleRootNode, "rectModeInHeightSlider")), areaData.inRect.height);
		this->setSliderValueWithMaxMin(*((ExSlider*)seekByName(singleRootNode, "rectModeOutHeight_Slider")), areaData.outRect.height);
		
		// �ر�����ͬ�� ������ �� ������
		((ExSlider*)seekByName(singleRootNode, "circleModeOutRadius_Slider"))->setEnabled(false);
		((ExSlider*)seekByName(singleRootNode, "circleModeInRadiusSlider"))->setEnabled(false);

		isDiyEditPolygonOrLines = false;
		((CheckBox*)seekByName(singleRootNode, "isDiyEditPolygonOrLinesCheckBox"))->setSelectedState(false);
		this->hideDiyFireAreaPoint();
	}
	else if (areaData.fAreaType == fireAreaType::circle ) {
		fireAreaMode->setSelectedIndex(1);

		seekByName(singleRootNode, "rectModeLayer")->setVisible(false);
		seekByName(singleRootNode, "circleModeLayer")->setVisible(true);
		seekByName(singleRootNode, "polygonAndLineCommonLayer")->setVisible(false);
		seekByName(singleRootNode, "polygonModeLayer")->setVisible(false);

		((ExSlider*)seekByName(singleRootNode, "edgeFireWidth_Slider"))->setEnabled(false);

		((ExSlider*)seekByName(singleRootNode, "circleModeOutRadius_Slider"))->setEnabled(true);
		((ExSlider*)seekByName(singleRootNode, "circleModeInRadiusSlider"))->setEnabled(true);
		this->setSliderValueWithMaxMin(*((ExSlider*)seekByName(singleRootNode, "circleModeInRadiusSlider")), areaData.inCircleRadius);
		this->setSliderValueWithMaxMin(*((ExSlider*)seekByName(singleRootNode, "circleModeOutRadius_Slider")), areaData.outCircleRadius);
		

		((ExSlider*)seekByName(singleRootNode, "rectModeOutWidth_Slider"))->setEnabled(false);
		((ExSlider*)seekByName(singleRootNode, "rectModeInWidthSlider"))->setEnabled(false);
		((ExSlider*)seekByName(singleRootNode, "rectModeOutHeight_Slider"))->setEnabled(false);
		((ExSlider*)seekByName(singleRootNode, "rectModeInHeightSlider"))->setEnabled(false);

		isDiyEditPolygonOrLines = false;
		((CheckBox*)seekByName(singleRootNode, "isDiyEditPolygonOrLinesCheckBox"))->setSelectedState(false);
		this->hideDiyFireAreaPoint();
	}
	else if (areaData.fAreaType == fireAreaType::polygon) {
		fireAreaMode->setSelectedIndex(2);

		seekByName(singleRootNode, "rectModeLayer")->setVisible(false);
		seekByName(singleRootNode, "circleModeLayer")->setVisible(false);
		seekByName(singleRootNode, "polygonAndLineCommonLayer")->setVisible(true);
		seekByName(singleRootNode, "polygonAndLineCommonLayer")->setSwallowTouches(false);
		seekByName(singleRootNode, "polygonModeLayer")->setVisible(true);

		((ExSlider*)seekByName(singleRootNode, "edgeFireWidth_Slider"))->setEnabled(true);
		this->setSliderValueWithMaxMin(*((ExSlider*)seekByName(singleRootNode, "edgeFireWidth_Slider")), areaData.edgeFireWidth);

		if (areaData.isEdgeFire) {
			((CheckBox*)seekByName(singleRootNode, "isEdgeFireCheckBox"))->setSelectedState(true);
		}
		else {
			((CheckBox*)seekByName(singleRootNode, "isEdgeFireCheckBox"))->setSelectedState(false);
		}

		if (nowEditingSignalPar->_fireArea.polygonPoints.size() <= 0) {
			nowEditingSignalPar->_fireArea.polygonPoints.push_back(Vec2(-50,0));
			nowEditingSignalPar->_fireArea.polygonPoints.push_back(Vec2(50, 0));
			nowEditingSignalPar->_fireArea.polygonPoints.push_back(Vec2(0, 90));
		}
		nowEditingSignalPar->_fireArea.isChangePolygonToTriangleVec = false;

		this->hideDiyFireAreaPoint();
		if (isDiyEditPolygonOrLines) {
			this->showDiyFireAreaPoint();
		}

	}
	else if (areaData.fAreaType == fireAreaType::lines) {
		fireAreaMode->setSelectedIndex(3);

		seekByName(singleRootNode, "rectModeLayer")->setVisible(false);
		seekByName(singleRootNode, "circleModeLayer")->setVisible(false);
		seekByName(singleRootNode, "polygonAndLineCommonLayer")->setVisible(true);
		seekByName(singleRootNode, "polygonModeLayer")->setVisible(false);

		((ExSlider*)seekByName(singleRootNode, "edgeFireWidth_Slider"))->setEnabled(true);
		this->setSliderValueWithMaxMin(*((ExSlider*)seekByName(singleRootNode, "edgeFireWidth_Slider")), areaData.edgeFireWidth);
		
		if (nowEditingSignalPar->_fireArea.linePoints.size() <= 0) {
			nowEditingSignalPar->_fireArea.linePoints.push_back(Vec2(-50, 0));
			nowEditingSignalPar->_fireArea.linePoints.push_back(Vec2(50, 0));
		}
		nowEditingSignalPar->_fireArea.isChangePolygonToTriangleVec = false;

		this->hideDiyFireAreaPoint();
		if (isDiyEditPolygonOrLines) {
			this->showDiyFireAreaPoint();
		}
	}

}


void ParticleUiView::update(float dt) {
	nowTime += dt;
	if (lastTouchDelay > 0) {
		lastTouchDelay -= dt;
	}
	else {
		lastTouchDelay = 0;
	}

	// ɾ�� diy ����༭�ĵ� ��ť
	if (isStartDeleteDiyFireAreaPointBtn) {
		if (deleteDiyFireAreaPointNeedTimeCount < deleteDiyFireAreaPointNeedTime) {
			deleteDiyFireAreaPointNeedTimeCount += dt;
		}
		else {
			deleteDiyFireAreaPointNeedTimeCount = 0;
			if (targetdiyFireAreaPointBtn) {
				deleteDiyFireAreaPoint(targetdiyFireAreaPointBtn);
				targetdiyFireAreaPointBtn = nullptr;
			}
			isStartDeleteDiyFireAreaPointBtn = false;
		}
	}
	else {
		deleteDiyFireAreaPointNeedTimeCount = 0;
	}

	// ɾ�� ���俪ʼʱ ������ �� ��ť
	if (isStartDeleteFireStartCurvePointBtn) {
		if (deleteFireStartCurvePointNeedTimeCount < deleteFireStartCurvePointNeedTime) {
			deleteFireStartCurvePointNeedTimeCount += dt;
		}
		else {
			deleteDiyFireAreaPointNeedTimeCount = 0;
			if (targetFireStartCurvePointBtn) {
				deleteFireStartCurvePoints(targetFireStartCurvePointBtn);
				targetFireStartCurvePointBtn = nullptr;
			}
			isStartDeleteFireStartCurvePointBtn = false;
		}
	}
	else {
		deleteFireStartCurvePointNeedTimeCount = 0;
	}

	// ɾ�� ������������������ ������ �� ��ť
	if (isStartDeleteParLifeCurvePointBtn) {
		if (deleteParLifeCurvePointNeedTimeCount < deleteParLifeCurvePointNeedTime) {
			deleteParLifeCurvePointNeedTimeCount += dt;
		}
		else {
			deleteParLifeCurvePointNeedTimeCount = 0;
			if (targetParLifeCurvePointBtn) {
				deleteParLifeCurvePoints(targetParLifeCurvePointBtn);
				targetParLifeCurvePointBtn = nullptr;
			}
			isStartDeleteParLifeCurvePointBtn = false;
		}
	}
	else {
		deleteParLifeCurvePointNeedTimeCount = 0;
	}

	// ɾ�� ���俪ʼʱ ����ɫ�� ������ �� ��ť
	if (isStartDeleteFireStartCurveColorBtn) {
		if (deleteFireStartCurveColorNeedTimeCount < deleteFireStartCurveColorNeedTime) {
			deleteFireStartCurveColorNeedTimeCount += dt;
		}
		else {
			deleteFireStartCurveColorNeedTimeCount = 0;
			if (targetFireStartCurveColorBtn) {
				deleteFireStartCurveColors(targetFireStartCurveColorBtn);
				targetFireStartCurveColorBtn = nullptr;
			}
			isStartDeleteFireStartCurveColorBtn = false;
		}
	}
	else {
		deleteFireStartCurveColorNeedTimeCount = 0;
	}

	// ɾ�� ������������ ����ɫ�� ������ �� ��ť
	if (isStartDeleteParLifeCurveColorBtn) {
		if (deleteParLifeCurveColorNeedTimeCount < deleteParLifeCurveColorNeedTime) {
			deleteParLifeCurveColorNeedTimeCount += dt;
		}
		else {
			deleteParLifeCurveColorNeedTimeCount = 0;
			if (targetParLifeCurveColorBtn) {
				deleteParLifeCurveColors(targetParLifeCurveColorBtn);
				targetParLifeCurveColorBtn = nullptr;
			}
			isStartDeleteParLifeCurveColorBtn = false;
		}
	}
	else {
		deleteParLifeCurveColorNeedTimeCount = 0;
	}

	// 
	//singlePar->setRotation(singlePar->getRotation() + 1);

}

// ��ʾ ��������� , ����κ��߶ε� , �������Զ���༭ʱ���� ������������� ת���� �㰴ť
void ParticleUiView::showDiyFireAreaPoint() {
	if (nowEditingSignalPar->_fireArea.fAreaType == fireAreaType::polygon && editMode == EditMode::single && singlePar) {
		auto itor = nowEditingSignalPar->_fireArea.polygonPoints.begin();
		while (itor != nowEditingSignalPar->_fireArea.polygonPoints.end()) {
			Vec2 pointPos = (*itor);

			auto diyFireAreaPoint = ui::Button::create(
				"coord_point21.png",
				"coord_point22.png");

			diyFireAreaPoint->setPosition(Point( pointPos.x, pointPos.y ));
			diyFireAreaPointVec.push_back(diyFireAreaPoint);
			diyFireAreaPoint->setSwallowTouches(false);
			singlePar->addChild(diyFireAreaPoint,400);
			diyFireAreaPoint->addTouchEventListener(CC_CALLBACK_2(ParticleUiView::diyFireAreaPointCall, this));

			itor++;
		}

	}
	else if (nowEditingSignalPar->_fireArea.fAreaType == fireAreaType::lines && editMode == EditMode::single && singlePar) {
		auto itor = nowEditingSignalPar->_fireArea.linePoints.begin();
		while (itor != nowEditingSignalPar->_fireArea.linePoints.end()) {
			Vec2 pointPos = (*itor);

			auto diyFireAreaPoint = ui::Button::create(
				"coord_point21.png",
				"coord_point22.png");

			diyFireAreaPoint->setPosition(Point(pointPos.x, pointPos.y));
			diyFireAreaPointVec.push_back(diyFireAreaPoint);
			diyFireAreaPoint->setSwallowTouches(false);
			singlePar->addChild(diyFireAreaPoint,400);
			diyFireAreaPoint->addTouchEventListener(CC_CALLBACK_2(ParticleUiView::diyFireAreaPointCall, this));

			itor++;
		}
	}
	if (nowEditingSignalPar->_fireArea.isChangePolygonToTriangleVec) {
		nowEditingSignalPar->_fireArea.isChangePolygonToTriangleVec = false;
	}
}

// ���� ���������
void ParticleUiView::hideDiyFireAreaPoint() {
	auto itor = diyFireAreaPointVec.begin();
	while (itor != diyFireAreaPointVec.end()) {
		(*itor)->removeFromParent();

		itor++;
	}
	diyFireAreaPointVec.clear();
}


void ParticleUiView::refreshDiyFireAreaPoints() {
	if (nowEditingSignalPar->_fireArea.fAreaType == fireAreaType::polygon && editMode == EditMode::single && singlePar) {
		if (nowEditingSignalPar->_fireArea.polygonPoints.size() == diyFireAreaPointVec.size()) {
			for (int i = 0; i < nowEditingSignalPar->_fireArea.polygonPoints.size(); ++i) {
				nowEditingSignalPar->_fireArea.polygonPoints.at(i) = diyFireAreaPointVec.at(i)->getPosition();
			}
		}
	}
	else if (nowEditingSignalPar->_fireArea.fAreaType == fireAreaType::lines && editMode == EditMode::single && singlePar) {
		if (nowEditingSignalPar->_fireArea.linePoints.size() == diyFireAreaPointVec.size()) {
			for (int i = 0; i < nowEditingSignalPar->_fireArea.linePoints.size(); ++i) {
				nowEditingSignalPar->_fireArea.linePoints.at(i) = diyFireAreaPointVec.at(i)->getPosition();
			}
		}
	}
	if (nowEditingSignalPar->_fireArea.isChangePolygonToTriangleVec) {
		nowEditingSignalPar->_fireArea.isChangePolygonToTriangleVec = false;
	}
}

void ParticleUiView::addDiyFireAreaPoint(Vec2 pos) {
	if (nowEditingSignalPar->_fireArea.fAreaType == fireAreaType::polygon && editMode == EditMode::single && singlePar) {
		// ԭ���ж�������Ƿ��� ������е�����һ�����ϣ������������һ���Ŀ��(û��ȣ�����ô���ܵ�õ�)��Ȼ���ٶ�Ӧ���� �����
		Vec2 realPos = Vec2(pos.x - singlePar->getPositionX(), pos.y - singlePar->getPositionY());
		int size = nowEditingSignalPar->_fireArea.polygonPoints.size();
		for (int i = 0; i < size; ++i) {
			Vec2 pointA, pointB;
			if (i == size - 1) {
				pointA = nowEditingSignalPar->_fireArea.polygonPoints.at(i);
				pointB = nowEditingSignalPar->_fireArea.polygonPoints.at(0);
			}
			else {
				pointA = nowEditingSignalPar->_fireArea.polygonPoints.at(i);
				pointB = nowEditingSignalPar->_fireArea.polygonPoints.at(i+1);
			}

			float arDis = pointA.getDistance(realPos);
			float abDis = pointA.getDistance(pointB);
			if (arDis > abDis) {
				continue;
			}
			float height = 5;  // ���¸�5������10
			float arAngle = Vec2(realPos.x - pointA.x, realPos.y - pointA.y).getAngle() / P_PI * 180;
			float abAngle = Vec2(pointB.x - pointA.x, pointB.y - pointA.y).getAngle() / P_PI * 180;
			float angleOffset = arAngle - abAngle;
			// ����ֱ�߳�������ˮƽ�ģ�����ʵ�Ĵ�������Ҫ��ת - abAngle ���ж�����ת��ĵ� �Ƿ���ˮƽ������
			Vec2 rectPos = Vec2(pointA.x + arDis * cosf( angleOffset / 180 * P_PI ) , pointA.y + arDis * sinf(angleOffset / 180 * P_PI));
			if (rectPos.x >= pointA.x && rectPos.x <= pointA.x + abDis && rectPos.y >= pointA.y - height && rectPos.y <= pointA.y + height) {
				if (i == size - 1) {
					nowEditingSignalPar->_fireArea.polygonPoints.push_back(realPos);
				}
				else {
					nowEditingSignalPar->_fireArea.polygonPoints.insert( nowEditingSignalPar->_fireArea.polygonPoints.begin() + i + 1 , realPos);
				}
				this->hideDiyFireAreaPoint();
				this->showDiyFireAreaPoint();
				break;
			}

		}

	}
	else if (nowEditingSignalPar->_fireArea.fAreaType == fireAreaType::lines && editMode == EditMode::single && singlePar) {
		Vec2 realPos = Vec2(pos.x - singlePar->getPositionX() , pos.y - singlePar->getPositionY());
		nowEditingSignalPar->_fireArea.linePoints.push_back(realPos);
		this->hideDiyFireAreaPoint();
		this->showDiyFireAreaPoint();
	}
}

void ParticleUiView::deleteDiyFireAreaPoint(ui::Button* btn) {
	if (nowEditingSignalPar->_fireArea.fAreaType == fireAreaType::polygon && editMode == EditMode::single && singlePar) {
		if (nowEditingSignalPar->_fireArea.polygonPoints.size() <= 3) {
			return;
		}

		auto itor = nowEditingSignalPar->_fireArea.polygonPoints.begin();
		while (itor != nowEditingSignalPar->_fireArea.polygonPoints.end()) {
			if ((*itor) == btn->getPosition()) {
				nowEditingSignalPar->_fireArea.polygonPoints.erase(itor);
				break;
			}
			itor++;
		}
		this->hideDiyFireAreaPoint();
		this->showDiyFireAreaPoint();
	}
	else if (nowEditingSignalPar->_fireArea.fAreaType == fireAreaType::lines && editMode == EditMode::single && singlePar) {
		if (nowEditingSignalPar->_fireArea.linePoints.size() <= 2) {
			return;
		}

		auto itor = nowEditingSignalPar->_fireArea.linePoints.begin();
		while (itor != nowEditingSignalPar->_fireArea.linePoints.end()) {
			if ((*itor) == btn->getPosition()) {
				nowEditingSignalPar->_fireArea.linePoints.erase(itor);
				break;
			}
			itor++;
		}
		this->hideDiyFireAreaPoint();
		this->showDiyFireAreaPoint();
	}
}

////////////////////********* ��������� ��ť �Ļص����� *******/////////////////////////////////////////
void ParticleUiView::diyFireAreaPointCall(cocos2d::Ref* pSender, ui::Widget::TouchEventType type) {
	switch (type) {
	case ui::Widget::TouchEventType::BEGAN:
		targetdiyFireAreaPointBtn = (ui::Button*)pSender;
		isStartDeleteDiyFireAreaPointBtn = true;
		break;
	case ui::Widget::TouchEventType::MOVED:
		isStartDeleteDiyFireAreaPointBtn = false;
		break;
	case ui::Widget::TouchEventType::ENDED:
		targetdiyFireAreaPointBtn = nullptr;
		isStartDeleteDiyFireAreaPointBtn = false;
		break;
	case ui::Widget::TouchEventType::CANCELED:
		targetdiyFireAreaPointBtn = nullptr;
		isStartDeleteDiyFireAreaPointBtn = false;
		break;
	default:
		break;
	}
}

////////////////////********* ��ʼ����ʱ���� ���� ��ť �Ļص����� *******/////////////////////////////////////////
void ParticleUiView::fireStartCurvePointCall(cocos2d::Ref* pSender, ui::Widget::TouchEventType type) {
	switch (type) {
	case ui::Widget::TouchEventType::BEGAN:
		targetFireStartCurvePointBtn = (ui::Button*)pSender;

		{
			auto btnItor = fireStartCurvePointBtnVec.begin();
			int index = 0;
			while (btnItor != fireStartCurvePointBtnVec.end()) {
				
				if ((*btnItor) == targetFireStartCurvePointBtn) {
					int size = fireStartCurvePointBtnVec.size();
					if(index == size -1)
						targetFireStartCurvePointBtn_right = fireStartCurvePointBtnVec.at(index);
					else
						targetFireStartCurvePointBtn_right = fireStartCurvePointBtnVec.at(index + 1);
					if (index == 0)
						targetFireStartCurvePointBtn_left = fireStartCurvePointBtnVec.at(index);
					else
						targetFireStartCurvePointBtn_left = fireStartCurvePointBtnVec.at(index - 1);
					break;
				}

				index++;
				btnItor++;
			}
		}

		isStartDeleteFireStartCurvePointBtn = true;
		break;
	case ui::Widget::TouchEventType::MOVED:
		isStartDeleteFireStartCurvePointBtn = false;
		break;
	case ui::Widget::TouchEventType::ENDED:
		targetFireStartCurvePointBtn = nullptr;
		isStartDeleteFireStartCurvePointBtn = false;
		targetFireStartCurvePointBtn_left = nullptr;
		targetFireStartCurvePointBtn_right = nullptr;
		break;
	case ui::Widget::TouchEventType::CANCELED:
		targetFireStartCurvePointBtn = nullptr;
		isStartDeleteFireStartCurvePointBtn = false;
		break;
	default:
		break;
	}
}

////////////////////********* ������������ ���� ���� ��ť �Ļص����� *******/////////////////////////////////////////
void ParticleUiView::parLifeCurvePointCall(cocos2d::Ref* pSender, ui::Widget::TouchEventType type) {
	switch (type) {
	case ui::Widget::TouchEventType::BEGAN:
		targetParLifeCurvePointBtn = (ui::Button*)pSender;

		{
			auto btnItor = parLifeCurvePointBtnVec.begin();
			int index = 0;
			while (btnItor != parLifeCurvePointBtnVec.end()) {

				if ((*btnItor) == targetParLifeCurvePointBtn) {
					int size = parLifeCurvePointBtnVec.size();
					if (index == size - 1)
						targetParLifeCurvePointBtn_right = parLifeCurvePointBtnVec.at(index);
					else
						targetParLifeCurvePointBtn_right = parLifeCurvePointBtnVec.at(index + 1);
					if (index == 0)
						targetParLifeCurvePointBtn_left = parLifeCurvePointBtnVec.at(index);
					else
						targetParLifeCurvePointBtn_left = parLifeCurvePointBtnVec.at(index - 1);
					break;
				}

				index++;
				btnItor++;
			}
		}

		isStartDeleteParLifeCurvePointBtn = true;
		break;
	case ui::Widget::TouchEventType::MOVED:
		isStartDeleteParLifeCurvePointBtn = false;
		break;
	case ui::Widget::TouchEventType::ENDED:
		targetParLifeCurvePointBtn = nullptr;
		isStartDeleteParLifeCurvePointBtn = false;
		targetParLifeCurvePointBtn_left = nullptr;
		targetParLifeCurvePointBtn_right = nullptr;
		break;
	case ui::Widget::TouchEventType::CANCELED:
		targetParLifeCurvePointBtn = nullptr;
		isStartDeleteParLifeCurvePointBtn = false;
		break;
	default:
		break;
	}
}

////////////////////********* ��ʼ����ʱ���� ����ɫ�� ���� ��ť �Ļص����� *******/////////////////////////////////////////
void ParticleUiView::fireStartCurveColorCall(cocos2d::Ref* pSender, ui::Widget::TouchEventType type) {
	switch (type) {
	case ui::Widget::TouchEventType::BEGAN:
		targetFireStartCurveColorBtn = (ui::Button*)pSender;

		isStartDeleteFireStartCurveColorBtn = true;
		break;
	case ui::Widget::TouchEventType::MOVED:
		isStartDeleteFireStartCurveColorBtn = false;
		break;
	case ui::Widget::TouchEventType::ENDED:
		targetFireStartCurveColorBtn = nullptr;
		isStartDeleteFireStartCurveColorBtn = false;
		break;
	case ui::Widget::TouchEventType::CANCELED:
		targetFireStartCurveColorBtn = nullptr;
		isStartDeleteFireStartCurveColorBtn = false;
		break;
	default:
		break;
	}
}

////////////////////********* ���������������� ����ɫ�� ���� ��ť �Ļص����� *******/////////////////////////////////////////
void ParticleUiView::parLifeCurveColorCall(cocos2d::Ref* pSender, ui::Widget::TouchEventType type) {
	switch (type) {
	case ui::Widget::TouchEventType::BEGAN:
		targetParLifeCurveColorBtn = (ui::Button*)pSender;

		isStartDeleteParLifeCurveColorBtn = true;
		break;
	case ui::Widget::TouchEventType::MOVED:
		isStartDeleteParLifeCurveColorBtn = false;
		break;
	case ui::Widget::TouchEventType::ENDED:
		targetParLifeCurveColorBtn = nullptr;
		isStartDeleteParLifeCurveColorBtn = false;
		break;
	case ui::Widget::TouchEventType::CANCELED:
		targetParLifeCurveColorBtn = nullptr;
		isStartDeleteParLifeCurveColorBtn = false;
		break;
	default:
		break;
	}
}

////////////////////********* �������ı� *******/////////////////////////////////////////
void ParticleUiView::onSliderChanged(Ref* sender, extension::Control::EventType) {
	CCLOG("-=-=-=-=--= slider changed");
	auto slider = (ExSlider*)sender;
	std::string sliderName = slider->getName();

	

	//�ҵ����������޸Ķ�Ӧ�ı༭��
	float start = sliderName.find("_", 0); // find ����-1 ����û���ҵ�
	if (start > -1) {
		std::string name = sliderName.substr(0, start);
		//����ж�Ӧ ������ ���������ı�������ֵ
		auto editBoxTem = seekByName(singleRootNode, name + "_Input");
		if (editBoxTem) {
			auto editBox = (ExEditBox*)editBoxTem;
			char str[20];
			sprintf(str, "%.2f", slider->getValue());
			editBox->setText(str);
		}
	}

	// �������������ı�
	if (sliderName == "maxParNum_Slider") {
		nowEditingSignalPar->_maxParticleNum = slider->getValue();
	}
	else if (sliderName == "duration_Slider") {
		nowEditingSignalPar->_duration = slider->getValue();
	}
	else if (sliderName == "fireRate_Slider") {
		nowEditingSignalPar->_emissionRate = slider->getValue();
		singlePar->stopSystem();
		singlePar->resetSystem();
	}
	else if (sliderName == "preFire_Slider") {
		nowEditingSignalPar->_preFireDuration = slider->getValue();
		singlePar->stopSystem();
		singlePar->resetSystem();
	}
	else if (sliderName == "fireDelay_Slider") {
		nowEditingSignalPar->_delayTime = slider->getValue();
		singlePar->stopSystem();
		singlePar->resetSystem();
	}
	//
	else if (sliderName == "rectModeOutWidth_Slider") {
		nowEditingSignalPar->_fireArea.outRect.width = slider->getValue();
		auto sliderVar = ((ExSlider*)seekByName(singleRootNode, "rectModeInWidthSlider"));
		sliderVar->setMaximumValueWithoutSetValue(slider->getValue());
		sliderVar->setValueWithoutChangedEvent(sliderVar->getValue());
		nowEditingSignalPar->_fireArea.inRect.width = sliderVar->getValue();
	}
	else if (sliderName == "rectModeInWidthSlider") {
		nowEditingSignalPar->_fireArea.inRect.width = slider->getValue();
	}
	else if (sliderName == "rectModeOutHeight_Slider") {
		nowEditingSignalPar->_fireArea.outRect.height = slider->getValue();
		auto sliderVar = ((ExSlider*)seekByName(singleRootNode, "rectModeInHeightSlider"));
		sliderVar->setMaximumValueWithoutSetValue(slider->getValue());
		sliderVar->setValueWithoutChangedEvent(sliderVar->getValue());
		nowEditingSignalPar->_fireArea.inRect.height = sliderVar->getValue();
	}
	else if (sliderName == "rectModeInHeightSlider") {
		nowEditingSignalPar->_fireArea.inRect.height = slider->getValue();
	}
	//
	else if (sliderName == "circleModeOutRadius_Slider") {
		nowEditingSignalPar->_fireArea.outCircleRadius = slider->getValue();
		auto sliderVar = ((ExSlider*)seekByName(singleRootNode, "circleModeInRadiusSlider"));
		sliderVar->setMaximumValueWithoutSetValue(slider->getValue());
		sliderVar->setValueWithoutChangedEvent(sliderVar->getValue());
		nowEditingSignalPar->_fireArea.inCircleRadius = sliderVar->getValue();
	}
	else if (sliderName == "circleModeInRadiusSlider") {
		nowEditingSignalPar->_fireArea.inCircleRadius = slider->getValue();
	}
	else if (sliderName == "edgeFireWidth_Slider") {
		nowEditingSignalPar->_fireArea.edgeFireWidth = slider->getValue();
	} 
	else if (sliderName == "oneValue_Slider") {
		nowEditingEmitterVarietyValue->constValue = slider->getValue();
	}
	else if (sliderName == "randValue1_Slider") {
		nowEditingEmitterVarietyValue->randConst1 = slider->getValue();
	}
	else if (sliderName == "randValue2_Slider") {
		nowEditingEmitterVarietyValue->randConst2 = slider->getValue();
	}

}


////////////////////********* �༭��ı� *******/////////////////////////////////////////
void ParticleUiView::editBoxEditingDidBegin(ExEditBox* editBox) {}
void ParticleUiView::editBoxEditingDidEnd(ExEditBox* editBox) {}
void ParticleUiView::editBoxTextChanged(ExEditBox* editBox, const std::string& text) {
	std::string editBoxName = editBox->getName();
	float value = std::atof(text.c_str());
	// ������벻������
	if(value == 0 && editBoxName != "fileNameInput")
		editBox->setText("0.0");


	//�ҵ�������
	float start = editBoxName.find("_", 0);
	if (start > -1) {
		std::string name = editBoxName.substr(0, start);
		//����ж�Ӧ�������Ļ���������ı们����������
		auto sliderTem = seekByName(singleRootNode, name + "_Slider");
		if (sliderTem) {
			auto slider = (ExSlider*)sliderTem;
			if (value > slider->getMaximumValue()) {
				slider->setMaximumValue(value);
			}
			else if (value < slider->getMinimumValue()) {
				slider->setMinimumValue(value);
			}
			slider->setValue(value);
		}
		// ����Ƕ��ֵ�����еı༭�� �� �ı�
		if (name == "moreConstant") {
			// ��� _ ������±�
			std::string indexStr = editBoxName.substr(start + 1, -1);
			int index = std::stoi(indexStr);
			nowEditingEmitterVarietyValue->constValues.at(index) = value;
		}
		if (name == "tagName") {
			// ��� _ ������±�
			std::string indexStr = editBoxName.substr(start + 1, -1);
			int index = std::stoi(indexStr);
			// ����id Ϊ index ��firePro��tagName
			singlePar->getFireProById(index)->_tagName = text;
			editBox->setText(text.c_str());
		}
		if (name == "localZorder") {
			// ��� _ ������±�
			std::string indexStr = editBoxName.substr(start + 1, -1);
			int index = std::stoi(indexStr);
			// ����id Ϊ index ��firePro��tagName
			singlePar->getFireProById(index)->_localZorder = value;
			//editBox->setText(text.c_str());

			singlePar->refreshZorder();
		}
		

		if (name == "fireStartCurveRandEditBox") {
			// ��� _ ������±�
			std::string indexStr = editBoxName.substr(start + 1, -1);
			int index = std::stoi(indexStr);
			nowEditingEmitterVarietyValue->curvePoints.at(index).z = value;
			nowEditingEmitterVarietyValue->isSetCurveKB = false;
		}
		if (name == "parLifeCurveRandEditBox") {
			// ��� _ ������±�
			std::string indexStr = editBoxName.substr(start + 1, -1);
			int index = std::stoi(indexStr);
			nowEditingEmitterVarietyValue->curvePoints.at(index).z = value;
			nowEditingEmitterVarietyValue->isSetCurveKB = false;
		}
		

	}

	// �������༭��ı�
	if (editBoxName == "textureAnchorPointX_Input") {
		nowEditingSignalPar->_texAnchorPoint.x = value;
	}
	else if (editBoxName == "textureAnchorPointY_Input") {
		nowEditingSignalPar->_texAnchorPoint.y = value;
	}
	else if (editBoxName == "topYInput_fireStart") {
		this->refreshFireStartCurvePoints(value);
		char valueStr[20];
		sprintf(valueStr, "-%.2f", value);
		((Text*)seekByName(singleRootNode, "downYWord_fireStart"))->setString(valueStr);
	}

	else if (editBoxName == "topYInput_parLife") {
		this->refreshParLifeCurvePoints(value);
		char valueStr[20];
		sprintf(valueStr, "-%.2f", value);
		((Text*)seekByName(singleRootNode, "downYWord_parLife"))->setString(valueStr);
	}
	else if (editBoxName == "leftXInput_parLife") {
		parLifeCurvePointLeftPrecent = value;
		if (parLifeCurvePointLeftPrecent >= parLifeCurvePointRightPrecent) {
			parLifeCurvePointLeftPrecent = parLifeCurvePointRightPrecent - 1;
			char valueStr[20];
			sprintf(valueStr, "%.2f", parLifeCurvePointLeftPrecent);
			editBox->setText(valueStr);
		}
		if (parLifeCurvePointLeftPrecent < 0) {
			parLifeCurvePointLeftPrecent = 0;
			editBox->setText("0");
		}
		this->refreshParLifeCurvePoints(parLifeCurvePointMaxHeight);
	}
	else if (editBoxName == "rightXInput_parLife") {
		parLifeCurvePointRightPrecent = value;
		if (parLifeCurvePointRightPrecent <= parLifeCurvePointLeftPrecent) {
			parLifeCurvePointRightPrecent = parLifeCurvePointLeftPrecent + 1;
			char valueStr[20];
			sprintf(valueStr, "%.2f", parLifeCurvePointRightPrecent);
			editBox->setText(valueStr);
		}
		if (parLifeCurvePointRightPrecent > 100) {
			parLifeCurvePointRightPrecent = 100;
			editBox->setText("100");
		}
		this->refreshParLifeCurvePoints(parLifeCurvePointMaxHeight);
	}
	else if (editBoxName == "R_ColorInput") {
		if (value > 255) {
			value = 255;
		}
		else if (value < 0) {
			value = 0;
		}
		rectColor.x = value;
		this->setPointerPosByRGB(Color3B(roundf(rectColor.x), roundf(rectColor.y), roundf(rectColor.z)));
	}
	else if (editBoxName == "G_ColorInput") {
		if (value > 255) {
			value = 255;
		}
		else if (value < 0) {
			value = 0;
		}
		rectColor.y = value;
		this->setPointerPosByRGB(Color3B(roundf(rectColor.x), roundf(rectColor.y), roundf(rectColor.z)));
	}
	else if (editBoxName == "B_ColorInput") {
		if (value > 255) {
			value = 255;
		}
		else if (value < 0) {
			value = 0;
		}
		rectColor.z = value;
		this->setPointerPosByRGB(Color3B(roundf(rectColor.x), roundf(rectColor.y), roundf(rectColor.z)));
	}

	else if (editBoxName == "randR_ColorInput") {
		if (nowEditingColorPoint) {
			nowEditingColorPoint->colorRand.x = value;
		}
	}
	else if (editBoxName == "randG_ColorInput") {
		if (nowEditingColorPoint) {
			nowEditingColorPoint->colorRand.y = value;
		}
	}
	else if (editBoxName == "randB_ColorInput") {
		if (nowEditingColorPoint) {
			nowEditingColorPoint->colorRand.z = value;
		}
	}
	else if (editBoxName == "parLifeProColor_leftSideInput") {
		if (value < 0) {
			value = 0;
			editBox->setText("0");
		}
		if (value >= parLifeCurveColorRightPrecent) {
			value = parLifeCurveColorRightPrecent - 1;
			char valueStr[20];
			sprintf(valueStr, "%.2f", value);
			editBox->setText(valueStr);
		}
		parLifeCurveColorLeftPrecent = value;
		this->refreshParLifeCurveColors();
	}
	else if (editBoxName == "parLifeProColor_rightSideInput") {
		if (value > 100) {
			value = 100;
			editBox->setText("100");
		}
		if (value <= parLifeCurveColorLeftPrecent) {
			value = parLifeCurveColorLeftPrecent + 1;
			char valueStr[20];
			sprintf(valueStr, "%.2f", value);
			editBox->setText(valueStr);
		}
		parLifeCurveColorRightPrecent = value;
		this->refreshParLifeCurveColors();
	}
	else if (editBoxName == "tail_fade_input") {
		nowEditingSignalPar->_tailPro.fade = value;
		//nowEditingSignalPar->_tailPro.refreshTailData();

		nowEditingSignalPar->_tailPro.tailNode->initWithFade(nowEditingSignalPar->_tailPro.fade, nowEditingSignalPar->_tailPro.minSeg, nowEditingSignalPar->_tailPro.stroke, nowEditingSignalPar->_tailPro.color, "tail/" + nowEditingSignalPar->_tailPro.tailName);
	}
	else if (editBoxName == "tail_minSeg_input") {
		nowEditingSignalPar->_tailPro.minSeg = value;
		//nowEditingSignalPar->_tailPro.refreshTailData();

		nowEditingSignalPar->_tailPro.tailNode->initWithFade(nowEditingSignalPar->_tailPro.fade, nowEditingSignalPar->_tailPro.minSeg, nowEditingSignalPar->_tailPro.stroke, nowEditingSignalPar->_tailPro.color, "tail/" + nowEditingSignalPar->_tailPro.tailName);
	}
	else if (editBoxName == "tail_stroke_input") {
		nowEditingSignalPar->_tailPro.stroke = value;
		//nowEditingSignalPar->_tailPro.refreshTailData();

		nowEditingSignalPar->_tailPro.tailNode->initWithFade(nowEditingSignalPar->_tailPro.fade, nowEditingSignalPar->_tailPro.minSeg, nowEditingSignalPar->_tailPro.stroke, nowEditingSignalPar->_tailPro.color, "tail/" + nowEditingSignalPar->_tailPro.tailName);
	}

}
void ParticleUiView::editBoxReturn(ExEditBox* editBox) {}

////////////////////********* �����¼� *******/////////////////////////////////////////
bool ParticleUiView::onTouchBegan(cocos2d::CCTouch* pTouch, cocos2d::CCEvent* pEvent) {
	Vec2 location = pTouch->getLocation();

	// �����ӱ༭ ��������ӷ����������ˣ����������һ��
	if (editMode == EditMode::single && singlePar) {
		if (!singlePar->getIsActive()) {
			if (nowEditingSignalPar) {
				nowEditingSignalPar->resetData();
			}
			singlePar->resetSystem();
			singlePar->setPosition(location);
		}
	}

	if (isOpenAddFilePanel) {
		auto filePanel = seekByName(mainRootNode, "addFilePanel");
		Vec2 pos = filePanel->getPosition();
		Size size = filePanel->getContentSize();

		if (location.x < pos.x || location.y < pos.y || location.x > pos.x + size.width || location.y > pos.y + size.height) {
			hideAddFilePanel();
		}
	}

	if (fabsf(lastTouchTime - nowTime) < 0.5 && lastTouchDelay <= 0) {
		// --------- ˫������
		
		// �� �Ǹ� ���������
		bool isAddFireStartCurvePoint = false;
		if (isOpenFireStartCurvePanel) {
			//���ж����Ǹ����������
			auto curvePanel = seekByName(singleRootNode, "curveMap_fireStart");
			Size curvePanelSize = curvePanel->getContentSize();
			Vec2 curvePanelPos = curvePanel->convertToWorldSpace(Vec2::ZERO);

			if (location.x >= curvePanelPos.x  && location.x <= curvePanelPos.x + curvePanelSize.width  &&
				location.y >= curvePanelPos.y  && location.y <= curvePanelPos.y + curvePanelSize.height) {
				// ��һ�����ߵ�
				this->addFireStartCurvePoints(location);
				isAddFireStartCurvePoint = true;
				CCLOG("+++++++++++++++++++++ addFireStartCurvePoints ��");
			}
		}
		// �� �Ǹ� ���������
		bool isAddParLifeCurvePoint = false;
		if (isOpenParLifeCurvePanel) {
			//���ж����Ǹ����������
			auto curvePanel = seekByName(singleRootNode, "curveMap_parLife");
			Size curvePanelSize = curvePanel->getContentSize();
			Vec2 curvePanelPos = curvePanel->convertToWorldSpace(Vec2::ZERO);

			if (location.x >= curvePanelPos.x  && location.x <= curvePanelPos.x + curvePanelSize.width  &&
				location.y >= curvePanelPos.y  && location.y <= curvePanelPos.y + curvePanelSize.height) {
				// ��һ�����ߵ�
				this->addParLifeCurvePoints(location);
				isAddParLifeCurvePoint = true;
				CCLOG("+++++++++++++++++++++ addParLifeCurvePoints ��");
			}
		}

		// ��������� �Զ���༭ ���������ܣ���˫������Ļ������ ����λ�ֱ�ߵ�
		if (isDiyEditPolygonOrLines && !isOpenFireStartCurvePanel && !isAddParLifeCurvePoint) {
			this->addDiyFireAreaPoint(location);
		}

		// ����� �������ڵ���ɫ����
		if (isOpenFireStartCurveColorPanel) {
			// �����˫����һ����ɫ��ť�����ɫ������
			if (targetFireStartCurveColorBtn) {
				
				for (int i = 0; i < fireStartCurveColorBtnVec.size(); ++i) {
					if (targetFireStartCurveColorBtn == fireStartCurveColorBtnVec.at(i)) {
						nowEditingColorPoint = &nowEditingEmitterColorValue->curveColors.at(i);
					}
				}
				
				this->showSetColorPanel(true);

				nowEditingColorNode = targetFireStartCurveColorBtn;
				this->setPointerPosByRGB(nowEditingColorNode->getColor());
			}

			// ���˫������ɫ��
			auto colorLine = seekByName(singleRootNode, "curveColorValueShowLine");
			Size colorLineSize = colorLine->getContentSize();
			Vec2 colorLinePos = colorLine->convertToWorldSpace(Vec2::ZERO);

			if (location.x >= colorLinePos.x  && location.x <= colorLinePos.x + colorLineSize.width  &&
				location.y >= colorLinePos.y  && location.y <= colorLinePos.y + colorLineSize.height) {

				this->addFireStartCurveColors(location.x);
			}

		}

		// ��� ���� �������������� �� ����ɫ�� ���߱༭
		if (isOpenParLifeCurveColorPanel) {
			// �����˫����һ����ɫ��ť�����ɫ������
			if (targetParLifeCurveColorBtn) {

				for (int i = 0; i < parLifeCurveColorBtnVec.size(); ++i) {
					if (targetParLifeCurveColorBtn == parLifeCurveColorBtnVec.at(i)) {
						nowEditingColorPoint = &nowEditingEmitterColorValue->curveColors.at(i);
					}
				}

				this->showSetColorPanel(true);

				nowEditingColorNode = targetParLifeCurveColorBtn;
				this->setPointerPosByRGB(nowEditingColorNode->getColor());
			}

			// ���˫������ɫ��
			auto colorLine = seekByName(singleRootNode, "parLifeProColorValueShowLine");
			Size colorLineSize = colorLine->getContentSize();
			Vec2 colorLinePos = colorLine->convertToWorldSpace(Vec2::ZERO);

			if (location.x >= colorLinePos.x  && location.x <= colorLinePos.x + colorLineSize.width  &&
				location.y >= colorLinePos.y  && location.y <= colorLinePos.y + colorLineSize.height) {

				this->addParLifeCurveColors(location.x);
			}

		}


		lastTouchDelay = 1.5;
	}

	lastTouchTime = nowTime;
	
	// ��ɫ��
	if (isOpenSetColorPanel) {
		// �жϴ������Ƿ��� ��ɫ������ 
		auto showRectColor = seekByName(singleRootNode, "showColorRect");
		Vec2 colorRectPos = showRectColor->convertToWorldSpace(Vec2::ZERO);
		Size colorRectSize = showRectColor->getContentSize();

		if (location.x >= colorRectPos.x && location.x <= colorRectPos.x + colorRectSize.width && location.y >= colorRectPos.y && location.y <= colorRectPos.y + colorRectSize.height) {
			isTouchInColorRect = true;
		}

		// �жϴ������Ƿ��� ��ɫ�� ��
		auto showLineColor = seekByName(singleRootNode, "showColorLine");
		Vec2 colorLinePos = showLineColor->convertToWorldSpace(Vec2::ZERO);
		Size colorLineSize = showLineColor->getContentSize();

		if (location.x >= colorLinePos.x && location.x <= colorLinePos.x + colorLineSize.width && location.y >= colorLinePos.y && location.y <= colorLinePos.y + colorLineSize.height) {
			isTouchInColorLine = true;
		}

		// �жϴ������Ƿ��� �ƶ��� ��
		auto moveBar = seekByName(singleRootNode, "setColorPanelMoveBar");
		Vec2 moveBarPos = moveBar->convertToWorldSpace(Vec2::ZERO);
		Size moveBarSize = moveBar->getContentSize();

		if (location.x >= moveBarPos.x && location.x <= moveBarPos.x + moveBarSize.width && location.y >= moveBarPos.y && location.y <= moveBarPos.y + moveBarSize.height) {
			isTouchInColorPanelMoveBar = true;
			 auto setColorPanel = seekByName(singleRootNode, "setColorPanel");
			touchMoveBarOffsetPos = Vec2(location.x - setColorPanel->getPosition().x , location.y - setColorPanel->getPosition().y);
		}

	}
	
	
	return true;
}
void ParticleUiView::onTouchMoved(cocos2d::CCTouch* pTouch, cocos2d::CCEvent* pEvent) {
	Vec2 location = pTouch->getLocation();
	 
	// �Ƿ������� ���������� �������
	bool isTouchInFireStartCurvePanel = false;
	if (isOpenFireStartCurvePanel) {
		auto curvePanel = seekByName(singleRootNode, "curveMap_fireStart");
		Size curvePanelSize = curvePanel->getContentSize();
		Vec2 curvePanelPos = curvePanel->convertToWorldSpace(Vec2::ZERO);
		if (location.x >= curvePanelPos.x  && location.x <= curvePanelPos.x + curvePanelSize.width  &&
			location.y >= curvePanelPos.y  && location.y <= curvePanelPos.y + curvePanelSize.height) {
			isTouchInFireStartCurvePanel = true;
		}
	}
	////���Ƿ����������������������������
	bool isTouchInParLifeCurvePanel = false;
	if (isOpenParLifeCurvePanel) {
		auto curvePanel = seekByName(singleRootNode, "curveMap_parLife");
		Size curvePanelSize = curvePanel->getContentSize();
		Vec2 curvePanelPos = curvePanel->convertToWorldSpace(Vec2::ZERO);
		if (location.x >= curvePanelPos.x  && location.x <= curvePanelPos.x + curvePanelSize.width  &&
			location.y >= curvePanelPos.y  && location.y <= curvePanelPos.y + curvePanelSize.height) {
			isTouchInParLifeCurvePanel = true;
		}
	}
	// �Ƿ������˷��������� "��ɫ" ���
	bool isTouchInFireStartColorLine = false;
	if (isOpenFireStartCurveColorPanel) {
		auto colorLine = seekByName(singleRootNode, "curveColorValuePanel_fireStart");
		Size colorLineSize = colorLine->getContentSize();
		Vec2 colorLinePos = colorLine->convertToWorldSpace(Vec2::ZERO);
		if (location.x >= colorLinePos.x  && location.x <= colorLinePos.x + colorLineSize.width  &&
			location.y >= colorLinePos.y  && location.y <= colorLinePos.y + colorLineSize.height) {
			isTouchInFireStartColorLine = true;
		}
	}
	// �Ƿ������� ������������ "��ɫ" ���
	bool isTouchInParLifeColorLine = false;
	if (isOpenParLifeCurveColorPanel) {
		auto colorLine = seekByName(singleRootNode, "parLifeProColorValuePanel");
		Size colorLineSize = colorLine->getContentSize();
		Vec2 colorLinePos = colorLine->convertToWorldSpace(Vec2::ZERO);
		if (location.x >= colorLinePos.x  && location.x <= colorLinePos.x + colorLineSize.width  &&
			location.y >= colorLinePos.y  && location.y <= colorLinePos.y + colorLineSize.height) {
			isTouchInParLifeColorLine = true;
		}
	}
	// �Ƿ������� ��ɫʰȡ����
	bool isTouchInSetColorPanel = false;
	if (isOpenSetColorPanel) {
		auto setColorPanel = seekByName(singleRootNode, "setColorPanel");
		Size setColorPanelSize = setColorPanel->getContentSize();
		Vec2 setColorPanelPos = setColorPanel->convertToWorldSpace(Vec2::ZERO);
		if (location.x >= setColorPanelPos.x  && location.x <= setColorPanelPos.x + setColorPanelSize.width  &&
			location.y >= setColorPanelPos.y  && location.y <= setColorPanelPos.y + setColorPanelSize.height) {
			isTouchInSetColorPanel = true;
		}
	}

	////////////////////////////////////////////
	bool isCanMovePar = true;

	if (isTouchInColorRect) {
		auto showRectColor = seekByName(singleRootNode, "showColorRect");
		Vec2 colorRectPosInPanel = showRectColor->getPosition();
		Vec2 colorRectPos = showRectColor->convertToWorldSpace(Vec2::ZERO);
		Size colorRectSize = showRectColor->getContentSize();

		Vec2 offsetPos = Vec2(location.x - colorRectPos.x, location.y - colorRectPos.y);
		if (offsetPos.x < 0) {
			offsetPos.x = 0;
		}
		else if (offsetPos.x > colorRectSize.width) {
			offsetPos.x = colorRectSize.width;
		}
		if (offsetPos.y < 0) {
			offsetPos.y = 0;
		}
		else if (offsetPos.y > colorRectSize.height) {
			offsetPos.y = colorRectSize.height;
		}
		this->setRectColor(offsetPos);
		seekByName(singleRootNode, "showColorRectPointer")->setPosition(Vec2(offsetPos.x + colorRectPosInPanel.x - colorRectSize.width / 2, offsetPos.y + colorRectPosInPanel.y - colorRectSize.height / 2));
		isCanMovePar = false;
	}
	else if (isTouchInColorLine) {
		auto showLineColor = seekByName(singleRootNode, "showColorLine");
		Vec2 colorLinePosInPanel = showLineColor->getPosition();
		Vec2 colorLinePos = showLineColor->convertToWorldSpace(Vec2::ZERO);
		Size colorLineSize = showLineColor->getContentSize();

		Vec2 offsetPos = Vec2(location.x - colorLinePos.x, location.y - colorLinePos.y);
		if (offsetPos.x < 0) {
			offsetPos.x = 0;
		}
		else if (offsetPos.x > colorLineSize.width) {
			offsetPos.x = colorLineSize.width;
		}
		if (offsetPos.y < 0) {
			offsetPos.y = 0;
		}
		else if (offsetPos.y > colorLineSize.height) {
			offsetPos.y = colorLineSize.height;
		}
		this->setLineColor(offsetPos.y);

		seekByName(singleRootNode, "showColorLinePointer")->setPositionY(offsetPos.y + colorLinePosInPanel.y - colorLineSize.height / 2);
		isCanMovePar = false;
	}
	else if (isTouchInColorPanelMoveBar) {
		auto setColorPanel = seekByName(singleRootNode, "setColorPanel");
		if (location.x < 0) {
			location.x = 0;
		}
		else if (location.x > Director::getInstance()->getVisibleSize().width) {
			location.x = Director::getInstance()->getVisibleSize().width;
		}
		if (location.y < 0) {
			location.y = 0;
		}
		else if (location.y > Director::getInstance()->getVisibleSize().height) {
			location.y = Director::getInstance()->getVisibleSize().height;
		}

		setColorPanel->setPosition(Vec2(location.x - touchMoveBarOffsetPos.x, location.y - touchMoveBarOffsetPos.y));
		isCanMovePar = false;
	}


	/////////////////////////////////////

	if (isDiyEditPolygonOrLines && !targetFireStartCurvePointBtn && !targetParLifeCurvePointBtn) {
		// ������ڱ༭��������
		if (targetdiyFireAreaPointBtn) {
			targetdiyFireAreaPointBtn->setPosition(Vec2(location.x - singlePar->getPositionX(), location.y - singlePar->getPositionY()));
			this->refreshDiyFireAreaPoints();
		}

		isCanMovePar = false;
	}
	else if (targetFireStartCurvePointBtn) {
		// --- ���ѡ����һ�� ���俪ʼ���� ������ ��ť
		// �ж�һ���Ƿ�Ϊ������������
		bool isLeftPoint = false;
		bool isRightPoint = false;
		if (targetFireStartCurvePointBtn == fireStartCurvePointBtnVec.at(0)) {
			isLeftPoint = true;
		}
		else if (targetFireStartCurvePointBtn == fireStartCurvePointBtnVec.at(fireStartCurvePointBtnVec.size() - 1)) {
			isRightPoint = true;
		}
		auto curvePanel = seekByName(singleRootNode, "curveMap_fireStart");
		Vec2 curvePanelPos = curvePanel->convertToWorldSpace(Vec2::ZERO);
		Size curvePanelSize = curvePanel->getContentSize();
		Vec2 realPos = Vec2(location.x - curvePanelPos.x  , location.y - curvePanelPos.y );
		// �Ƿ�Ϊ����������
		if (isLeftPoint) {
			realPos.x = 0;
		}
		if (isRightPoint) {
			realPos.x = curvePanel->getContentSize().width;
		}
		// ���Ƶ㲻����
		realPos.y = (realPos.y > curvePanelSize.height ) ? curvePanelSize.height : realPos.y;
		realPos.y = (realPos.y < 0) ? 0 : realPos.y;
		realPos.x = (realPos.x > curvePanelSize.width ) ? curvePanelSize.width : realPos.x;
		realPos.x = (realPos.x < 0) ? 0 : realPos.x;
		// ���Ƶ� �� X���겻�� ����������
		float leftX = targetFireStartCurvePointBtn_left->getPositionX();
		float rightX = targetFireStartCurvePointBtn_right->getPositionX();
		if (realPos.x < leftX) {
			realPos.x = leftX;
		}
		if (realPos.x > rightX) {
			realPos.x = rightX;
		}

		targetFireStartCurvePointBtn->setPosition(realPos);
		this->refreshFireStartCurvePoints(fireStartCurvePointMaxHeight);

		isCanMovePar = false;
	}
	else if (targetParLifeCurvePointBtn) {
		// --- ���ѡ����һ�� ������������������ ������ ��ť
		// �ж�һ���Ƿ�Ϊ������������
		bool isLeftPoint = false;
		bool isRightPoint = false;
		if (targetParLifeCurvePointBtn == parLifeCurvePointBtnVec.at(0)) {
			isLeftPoint = true;
		}
		else if (targetParLifeCurvePointBtn == parLifeCurvePointBtnVec.at(parLifeCurvePointBtnVec.size() - 1)) {
			isRightPoint = true;
		}
		auto curvePanel = seekByName(singleRootNode, "curveMap_parLife");
		Vec2 curvePanelPos = curvePanel->convertToWorldSpace(Vec2::ZERO);
		Size curvePanelSize = curvePanel->getContentSize();
		Vec2 realPos = Vec2(location.x - curvePanelPos.x, location.y - curvePanelPos.y);
		// �Ƿ�Ϊ����������
		if (isLeftPoint) {
			realPos.x = 0;
		}
		if (isRightPoint) {
			realPos.x = curvePanel->getContentSize().width;
		}
		// ���Ƶ㲻����
		realPos.y = (realPos.y > curvePanelSize.height) ? curvePanelSize.height : realPos.y;
		realPos.y = (realPos.y < 0) ? 0 : realPos.y;
		realPos.x = (realPos.x > curvePanelSize.width) ? curvePanelSize.width : realPos.x;
		realPos.x = (realPos.x < 0) ? 0 : realPos.x;
		// ���Ƶ� �� X���겻�� ����������
		float leftX = targetParLifeCurvePointBtn_left->getPositionX();
		float rightX = targetParLifeCurvePointBtn_right->getPositionX();
		if (realPos.x < leftX) {
			realPos.x = leftX;
		}
		if (realPos.x > rightX) {
			realPos.x = rightX;
		}

		targetParLifeCurvePointBtn->setPosition(realPos);
		this->refreshParLifeCurvePoints(parLifeCurvePointMaxHeight);

		isCanMovePar = false;
	}
	else if (isTouchInFireStartCurvePanel) {
		isCanMovePar = false;
	}
	else if (isTouchInParLifeCurvePanel) {
		isCanMovePar = false;
	}
	else if (targetFireStartCurveColorBtn) {
		auto colorLine = seekByName(singleRootNode, "curveColorValueShowLine");
		Vec2 colorLinePos = colorLine->convertToWorldSpace(Vec2::ZERO);
		Size colorLineSize = colorLine->getContentSize();
		float realPosX = location.x - colorLinePos.x;

		realPosX = (realPosX < 0) ? 0 : realPosX;
		realPosX = (realPosX > colorLineSize.width) ? colorLineSize.width : realPosX;

		targetFireStartCurveColorBtn->setPositionX(realPosX);
		this->refreshFireStartCurveColors();

		isCanMovePar = false;
	}
	else if (targetParLifeCurveColorBtn) {
		auto colorLine = seekByName(singleRootNode, "parLifeProColorValueShowLine");
		Vec2 colorLinePos = colorLine->convertToWorldSpace(Vec2::ZERO);
		Size colorLineSize = colorLine->getContentSize();
		float realPosX = location.x - colorLinePos.x;

		realPosX = (realPosX < 0) ? 0 : realPosX;
		realPosX = (realPosX > colorLineSize.width) ? colorLineSize.width : realPosX;
		// ���������㣬���ܶ�
		if (targetParLifeCurveColorBtn == parLifeCurveColorBtnVec.at(0)) {
			realPosX = 0;
		}
		else if (targetParLifeCurveColorBtn == parLifeCurveColorBtnVec.at(parLifeCurveColorBtnVec.size() - 1)) {
			realPosX = colorLineSize.width;
		}

		targetParLifeCurveColorBtn->setPositionX(realPosX);
		this->refreshParLifeCurveColors();

		isCanMovePar = false;
	}

	//else 
	else if (isTouchInFireStartColorLine) {
		isCanMovePar = false;
	}
	else if (isTouchInParLifeColorLine) {
		isCanMovePar = false;
	}
	else if (isTouchInSetColorPanel) {
		isCanMovePar = false;
	}

	if (isCanMovePar) {
		
		if (editMode == EditMode::single && singlePar) {
			singlePar->setPosition(location);
		}
	}

}
void ParticleUiView::onTouchEnded(cocos2d::CCTouch* pTouch, cocos2d::CCEvent* pEvent) {
	isTouchInColorRect = false;
	isTouchInColorLine = false;
	isTouchInColorPanelMoveBar = false;
	touchMoveBarOffsetPos = Vec2(0, 0);
}

////////////////////*********�����б�Ļص�����*******/////////////////////////////////////////
void ParticleUiView::onDropDownList(Object* list, ui::Widget::TouchEventType type, DropDownList* sender) {
	switch (type) {
	case ui::Widget::TouchEventType::ENDED:
		sender->onSelected(list); //�ر������б��

		std::string name = sender->getName();
		int index = dynamic_cast<ui::Button*>(list)->getTag();
		if (name == "sourceBlend") {
			nowEditingSignalPar->_sourceBlend = getBlendTypeByInt(index);
		}
		else if (name == "destBlend") {
			nowEditingSignalPar->_destBlend = getBlendTypeByInt(index);
		}
		else if (name == "textureList") {
			nowEditingSignalPar->_texName = "texture/" + texNames.at(index);

		}
		else if (name == "jsonFileList") {
			ParticleEmitter::fireProId = 1;

			nowFileName = "json/" + fileNames.at(index);
			singlePar->stopSystem();
			singlePar->clearData();
			this->clearMoreParPanel();
			//nowEditingSignalPar->resetData();
			//nowEditingSignalPar->readJsonData( FileCenter::getInstance()->readJsonData(nowFileName) );
			singlePar->readJsonDataFromFile(nowFileName);
			nowEditingSignalPar = singlePar->getFirstFirePro();
			this->initMoreParPanel();
			singlePar->resetSystem();
			if (nowEditingSignalPar) {
				this->initSingleParUi();
			}
			// �ص����е����
			this->setTailProPanelVisible(false);
			this->setFireProLayerVisible(false);
			isSingleModeFireProLayerOpen = false;
			this->hideFireStartProValuePanel();
			this->hideParLifeProValuePanel();
		}
		else if (name == "addFireProjsonFileList") {
			nowAddFireProJsonFileName = addFireProJsonfileNames.at(index);

		}
		else if (name == "refSpriteList") {
			if (refSprite) {
				refSprite->removeFromParent();
				refSprite = nullptr;
			}
			if (!refSprite && index != 0) {
				refSprite = Sprite::create("refSprite/" + refSpriteNames.at(index-1));
				refSprite->setPosition(Director::getInstance()->getVisibleSize().width/2 , Director::getInstance()->getVisibleSize().height / 2);
				Node* topNode = this;
				while (true) {
					if (topNode->getParent() && topNode->getParent()->isRunning()) {
						topNode = topNode->getParent();
					}
					else {
						break;
					}
				}

				topNode->addChild(refSprite, -100);
			}
		}
		else if (name == "tailSpriteList") {
			nowEditingSignalPar->_tailPro.tailName = tailSpriteNames.at(index);
			//nowEditingSignalPar->_tailPro.refreshTailData();

			nowEditingSignalPar->_tailPro.tailNode->initWithFade(nowEditingSignalPar->_tailPro.fade, nowEditingSignalPar->_tailPro.minSeg, nowEditingSignalPar->_tailPro.stroke, nowEditingSignalPar->_tailPro.color, "tail/" + nowEditingSignalPar->_tailPro.tailName);
		}
		else if (name == "fireAreaMode") {
			if (index == 0) {
				nowEditingSignalPar->_fireArea.fAreaType = fireAreaType::rect;
				this->setFireAreaMode(nowEditingSignalPar->_fireArea);
			}
			else if (index == 1) {
				nowEditingSignalPar->_fireArea.fAreaType = fireAreaType::circle;
				this->setFireAreaMode(nowEditingSignalPar->_fireArea);
			}
			else if (index == 2) {
				nowEditingSignalPar->_fireArea.fAreaType = fireAreaType::polygon;
				this->setFireAreaMode(nowEditingSignalPar->_fireArea);
			}
			else if (index == 3) {
				nowEditingSignalPar->_fireArea.fAreaType = fireAreaType::lines;
				this->setFireAreaMode(nowEditingSignalPar->_fireArea);
			}
		}
		else if (name == "fireStartProValueType") {
			if (index == 0) {
				if (nowSelectedSingleProType == singleProType::color_fireStart) {
					this->showFireStartProColorValuePanel(singleProType::color_fireStart, emitterPropertyType::oneConstant);
				}
				else {
					this->showFireStartProValuePanel(nowSelectedSingleProType, emitterPropertyType::oneConstant);
				}
			}
			else if (index == 1) {
				if (nowSelectedSingleProType == singleProType::color_fireStart) {
					this->showFireStartProColorValuePanel(singleProType::color_fireStart, emitterPropertyType::randBetweenTwoConst);
				}
				else {
					this->showFireStartProValuePanel(nowSelectedSingleProType, emitterPropertyType::randBetweenTwoConst);
				}
				
			}
			else if (index == 2) {
				if (nowSelectedSingleProType == singleProType::color_fireStart) {
					this->showFireStartProColorValuePanel(singleProType::color_fireStart, emitterPropertyType::moreConstant);
				}
				else {
					this->showFireStartProValuePanel(nowSelectedSingleProType, emitterPropertyType::moreConstant);
				}
				
			}
			else if (index == 3) {
				if (nowSelectedSingleProType == singleProType::color_fireStart) {
					this->showFireStartProColorValuePanel(singleProType::color_fireStart, emitterPropertyType::curve);
				}
				else {
					this->showFireStartProValuePanel(nowSelectedSingleProType, emitterPropertyType::curve);
				}

				
			}
			
		}
		//else if (name == "jsonFileList") {
		//	jsonFile = "json/" + jsonFiles.at(index);

		//	/////�ض�����
		//	this->readJsonData();

		//}
		//else if (name == "bgSpriteList") {
		//	if (gameBg) {
		//		gameBg->removeFromParent();

		//		gameBg = Sprite::create("bg/" + bgSprites.at(index));
		//		gameBg->setAnchorPoint(Point(0, 0));
		//		gameBg->setPosition(0, 0);
		//		gameBg->setScaleX(Director::getInstance()->getWinSize().width / gameBg->getContentSize().width);
		//		gameBg->setScaleY(Director::getInstance()->getWinSize().height / gameBg->getContentSize().height);
		//		this->addChild(gameBg, -1000);
		//	}

		//}
	

	}
}

////////////////////********* ��ѡ��ص����� *******/////////////////////////////////////////
void ParticleUiView::checkBoxEvent(Ref* pSender, ui::CheckBox::EventType type) {
	auto checkBox = (ui::CheckBox*)pSender;
	std::string name = checkBox->getName();

	switch (type)
	{
	case ui::CheckBox::EventType::SELECTED:
		/*if (name == "singleParMode") {
			((CheckBox*)seekByName(mainRootNode, "moreParMode"))->setSelectedState(false);
			seekByName(mainRootNode, "singleParModeFilePanel")->setVisible(true);
			seekByName(mainRootNode, "moreParModeFilePanel")->setVisible(false);
			seekByName(mainRootNode, "moreParModeProPanel")->setVisible(false);

			editMode = EditMode::single;
		}
		if (name == "moreParMode") {
			((CheckBox*)seekByName(mainRootNode, "singleParMode"))->setSelectedState(false);
			seekByName(mainRootNode, "singleParModeFilePanel")->setVisible(false);
			seekByName(mainRootNode, "moreParModeFilePanel")->setVisible(true);
			seekByName(mainRootNode, "moreParModeProPanel")->setVisible(true);

			editMode = EditMode::more;
		}*/
		if (name == "isLoopCheckBox") {
			nowEditingSignalPar->_isLoop = true;
			if (editMode == EditMode::single && singlePar && nowEditingSignalPar) {
				if (!singlePar->getIsActive()) {
					nowEditingSignalPar->resetData();
					singlePar->resetSystem();
				}
			}
		}
		if (name == "moveMode_free") {
			((CheckBox*)seekByName(singleRootNode, "moveMode_relative"))->setSelectedState(false);
			
			// ��Ҫ ��render �����ټӽ���
			singlePar->clearRender();
			singlePar->resetSystem();
			nowEditingSignalPar->_positionType = positionType::FREE;

			singlePar->addRender(true);
			
		}
		if (name == "moveMode_relative") {
			((CheckBox*)seekByName(singleRootNode, "moveMode_free"))->setSelectedState(false);
			
			// ��Ҫ ��render �����ټӽ���
			singlePar->clearRender();
			singlePar->resetSystem();
			nowEditingSignalPar->_positionType = positionType::RELATIVE;

			singlePar->addRender(true);
		}
		if (name == "angleMode_global") {
			((CheckBox*)seekByName(singleRootNode, "angleMode_local"))->setSelectedState(false);
			nowEditingSignalPar->_angleType = fireAngleType::global;
		}
		if (name == "angleMode_local") {
			((CheckBox*)seekByName(singleRootNode, "angleMode_global"))->setSelectedState(false);
			nowEditingSignalPar->_angleType = fireAngleType::local;
		}
		if (name == "isUseDrawNodeCheckBox") {
			pp::ParticleEmitter::isUseDrawNode = true;
		}
		if (name == "isEdgeFireCheckBox") {
			nowEditingSignalPar->_fireArea.isEdgeFire = true;
		}
		if (name == "isDiyEditPolygonOrLinesCheckBox") {
			isDiyEditPolygonOrLines = true;
			this->showDiyFireAreaPoint();
		}
		if (name == "isOpenParLifeProCheckBox") {
			if (nowEditingEmitterVarietyValue->curvePoints.size() < 2) {
				nowEditingEmitterVarietyValue->curvePoints.clear();
				nowEditingEmitterVarietyValue->curvePoints.push_back(Vec3(0, 0, 0));
				nowEditingEmitterVarietyValue->curvePoints.push_back(Vec3(100, 10, 0));
			}
			this->hideParLifeCurvePoints();
			this->showParLifeCurvePoints();
		}
		if (name == "isOpenParLifeProColorCheckBox") {
			if (nowEditingEmitterColorValue->curveColors.size() < 2) {
				nowEditingEmitterColorValue->curveColors.clear();
				colorCurvePoint colorLeft;
				colorLeft.x = 0;
				colorLeft.colorY = Color3B(255, 255, 255);
				colorCurvePoint colorRight;
				colorRight.x = 100;
				colorRight.colorY = Color3B(255, 255, 255);

				nowEditingEmitterColorValue->curveColors.push_back(colorLeft);
				nowEditingEmitterColorValue->curveColors.push_back(colorRight);
			}
			this->hideParLifeCurveColors();
			this->showParLifeCurveColors();
		}
		if (name == "tail_active_checkbox") {
			nowEditingSignalPar->_tailPro.isActive = true;
			nowEditingSignalPar->_tailPro.tailName = tailSpriteNames.at(0);
		}
		if (name == "addRenderFireProCheckBox") {
			((CheckBox*)seekByName(mainRootNode, "addEmitterFireProCheckBox"))->setSelectedState(false);
		}
		if (name == "addEmitterFireProCheckBox") {
			((CheckBox*)seekByName(mainRootNode, "addRenderFireProCheckBox"))->setSelectedState(false);
		}
		
		break;
	case ui::CheckBox::EventType::UNSELECTED:
		/*if (name == "singleParMode") {
			((CheckBox*)seekByName(mainRootNode, "moreParMode"))->setSelectedState(true);
			seekByName(mainRootNode, "singleParModeFilePanel")->setVisible(false);
			seekByName(mainRootNode, "moreParModeFilePanel")->setVisible(true);
			seekByName(mainRootNode, "moreParModeProPanel")->setVisible(true);

			editMode = EditMode::more;
		}
		if (name == "moreParMode") {
			((CheckBox*)seekByName(mainRootNode, "singleParMode"))->setSelectedState(true);
			seekByName(mainRootNode, "singleParModeFilePanel")->setVisible(true);
			seekByName(mainRootNode, "moreParModeFilePanel")->setVisible(false);
			seekByName(mainRootNode, "moreParModeProPanel")->setVisible(false); 

			editMode = EditMode::single;
		}*/
		if (name == "isLoopCheckBox") {
			nowEditingSignalPar->_isLoop = false;
		}
		if (name == "moveMode_free") {
			((CheckBox*)seekByName(singleRootNode, "moveMode_relative"))->setSelectedState(true);
			
			// ��Ҫ ��render �����ټӽ���
			singlePar->clearRender();
			singlePar->resetSystem();
			nowEditingSignalPar->_positionType = positionType::RELATIVE;

			singlePar->addRender(true);
		}
		if (name == "moveMode_relative") {
			((CheckBox*)seekByName(singleRootNode, "moveMode_free"))->setSelectedState(true);
			
			// ��Ҫ ��render �����ټӽ���
			singlePar->clearRender();
			singlePar->resetSystem();
			nowEditingSignalPar->_positionType = positionType::FREE;

			singlePar->addRender(true);
		}
		if (name == "angleMode_global") {
			((CheckBox*)seekByName(singleRootNode, "angleMode_local"))->setSelectedState(true);
			nowEditingSignalPar->_angleType = fireAngleType::local;
		}
		if (name == "angleMode_local") {
			((CheckBox*)seekByName(singleRootNode, "angleMode_global"))->setSelectedState(true);
			nowEditingSignalPar->_angleType = fireAngleType::global;
		}
		if (name == "isUseDrawNodeCheckBox") {
			pp::ParticleEmitter::isUseDrawNode = false;
		}
		if (name == "isEdgeFireCheckBox") {
			nowEditingSignalPar->_fireArea.isEdgeFire = false;
		}
		if (name == "isDiyEditPolygonOrLinesCheckBox") {
			isDiyEditPolygonOrLines = false;
			this->hideDiyFireAreaPoint();
		}
		if (name == "isOpenParLifeProCheckBox") {
			
			nowEditingEmitterVarietyValue->curvePoints.clear();
			
			this->hideParLifeCurvePoints();
			this->showParLifeCurvePoints();
		}
		if (name == "isOpenParLifeProColorCheckBox") {
			nowEditingEmitterColorValue->curveColors.clear();

			this->hideParLifeCurveColors();
			this->showParLifeCurveColors();
		}
		if (name == "tail_active_checkbox") {
			nowEditingSignalPar->_tailPro.isActive = false;
		}
		if (name == "addRenderFireProCheckBox") {
			((CheckBox*)seekByName(mainRootNode, "addEmitterFireProCheckBox"))->setSelectedState(true);
		}
		if (name == "addEmitterFireProCheckBox") {
			((CheckBox*)seekByName(mainRootNode, "addRenderFireProCheckBox"))->setSelectedState(true);
		}

		break;
	default:
		break;
	}
}	


// ��ȡ texture�ļ��µ� .png ����ͼƬ
std::vector<std::string> ParticleUiView::getPngFileName() {
	std::vector<std::string> texList;
	const int maxNum = 300;
	char pngList[maxNum][100];
	int i = 0;

	string path = ""; //FileUtils::getInstance()->getWritablePath();
	path.append("texture/*.png");
	WIN32_FIND_DATAA wfd;
	HANDLE hFind;

	for (int i = 0; i < path.size(); ++i)
	{
		if ('/' == path[i])
			path[i] = '\\';
	}

	hFind = FindFirstFileA((LPCSTR)path.c_str(), &wfd);
	do {
		strcpy(pngList[i], (char*)wfd.cFileName);
		texList.push_back(pngList[i]);
		if (++i >= maxNum) break;
	} while (FindNextFileA(hFind, &wfd));
	FindClose(hFind);

	return texList;
}


// ��ȡ texture�ļ��µ� .png ����ͼƬ
std::vector<std::string> ParticleUiView::getJsonFileName() {
	std::vector<std::string> texList;
	const int maxNum = 300;
	char pngList[maxNum][100];
	int i = 0;

	string path = ""; //FileUtils::getInstance()->getWritablePath();
	path.append("json/*.par");
	WIN32_FIND_DATAA wfd;
	HANDLE hFind;

	for (int i = 0; i < path.size(); ++i)
	{
		if ('/' == path[i])
			path[i] = '\\';
	}

	hFind = FindFirstFileA((LPCSTR)path.c_str(), &wfd);
	do {
		strcpy(pngList[i], (char*)wfd.cFileName);
		texList.push_back(pngList[i]);
		if (++i >= maxNum) break;
	} while (FindNextFileA(hFind, &wfd));
	FindClose(hFind);

	return texList;
}


std::vector<std::string> ParticleUiView::getRefSpriteName() {
	std::vector<std::string> texList;
	const int maxNum = 300;
	char pngList[maxNum][100];
	int i = 0;

	string path = ""; //FileUtils::getInstance()->getWritablePath();
	path.append("refSprite/*.png");
	WIN32_FIND_DATAA wfd;
	HANDLE hFind;

	for (int i = 0; i < path.size(); ++i)
	{
		if ('/' == path[i])
			path[i] = '\\';
	}

	hFind = FindFirstFileA((LPCSTR)path.c_str(), &wfd);
	do {
		strcpy(pngList[i], (char*)wfd.cFileName);
		texList.push_back(pngList[i]);
		if (++i >= maxNum) break;
	} while (FindNextFileA(hFind, &wfd));
	FindClose(hFind);

	return texList;
}

std::vector<std::string> ParticleUiView::getTailSpriteName() {
	std::vector<std::string> texList;
	const int maxNum = 300;
	char pngList[maxNum][100];
	int i = 0;

	string path = ""; //FileUtils::getInstance()->getWritablePath();
	path.append("tail/*.png");
	WIN32_FIND_DATAA wfd;
	HANDLE hFind;

	for (int i = 0; i < path.size(); ++i)
	{
		if ('/' == path[i])
			path[i] = '\\';
	}

	hFind = FindFirstFileA((LPCSTR)path.c_str(), &wfd);
	do {
		strcpy(pngList[i], (char*)wfd.cFileName);
		texList.push_back(pngList[i]);
		if (++i >= maxNum) break;
	} while (FindNextFileA(hFind, &wfd));
	FindClose(hFind);

	return texList;
}

