#include "stdheader.h"
#include "SimpleUI.h"

template <typename T>
bool getWidgetSize( MyGUI::Widget* pw,MyGUI::IntSize& size ){
	T* p = pw->castType<T>(false);
	
	if( p ){
		size = CalcTextWidth(p->getCaption(),p->getFontName());
		return true;
	}
	return false;
}

static bool getComboBoxSize( MyGUI::Widget* pw,MyGUI::IntSize& size ){
	MyGUI::ComboBox* p = pw->castType<MyGUI::ComboBox>(false);
	
	if( p ){
		MyGUI::IntSize s,ss;
		for( size_t i=0;i<p->getItemCount();++i ){
			s = CalcTextWidth(p->getItemNameAt(i),p->getFontName());
			ss.width = max(s.width,ss.width);
		}
		s = CalcTextWidth(p->getCaption(),p->getFontName());
		size.width = max(s.width,ss.width) + 26;
		size.height = max(s.height,ss.height)+4;
		return true;
	}
	return false;
}

MyGUI::IntSize CalcTextWidth( MyGUI::UString text,string font ){
	MyGUI::FontManager* pfm = MyGUI::FontManager::getInstancePtr();
	MyGUI::IFont* pf = pfm->getByName(font);
	MyGUI::IntSize size;
	size.height = pf->getDefaultHeight();
	if( pf ){
		for( MyGUI::UString::iterator i=text.begin();i!=text.end();++i ){
			MyGUI::GlyphInfo* pg = pf->getGlyphInfo(*i);
			if( pg ){
				size.width += (int)pg->width;
			}
		}
	}
	return size;
}

MyGUI::IntSize CalcWidgetSize(MyGUI::Widget* pw){
	MyGUI::IntSize size;
	if( pw ){
		if( getComboBoxSize(pw,size) )
			return size;
		if( getWidgetSize<MyGUI::EditBox>(pw,size) ){
			size.width += 8;
			size.height += 4;
			return size;
		}if( getWidgetSize<MyGUI::TextBox>(pw,size) )
			return size;
	}
	return size;
}

/*重新布局窗口
*/
void Grid::reLayout(){
	assert(parent && column > 0 );
	int i,x,y,width,height;
	MyGUI::IntSize size;
	vector<int> mh((int)(parent->getChildCount()/column)+1),mv(column);
	MyGUI::ScrollView *psw = parent->castType<MyGUI::ScrollView>(false);

	fill(mh.begin(),mh.end(),0);
	fill(mv.begin(),mv.end(),0);
	for( i = 0;i<(int)parent->getChildCount();++i ){
		MyGUI::Widget* pchild = parent->getChildAt(i);
		MyGUI::IntSize size = pchild->getSize();
		//如果没有设置这里自动计算
		if( size.width==0 ){
			size.width = CalcWidgetSize(pchild).width;
		}
		if( size.height==0 ){
			size.height = CalcWidgetSize(pchild).height;
		}
		if( size.width>mv[i%column] )
			mv[i%column] = size.width;
		if( size.height>mh[(int)(i/column)] )
			mh[(int)(i/column)] = size.height;
	}
	width = sum(mv,mv.size())+(column-1)*space;
	height = sum(mh,mh.size());
	size = parent->getSize();
	//对于ScrollView可能有Bug，这里减去24感觉正好
	if( psw ){
		size.width -= 24;
		size.height -= 24;
	}
	//不要小于0
	x = max((int)((size.width-width)/2),0);
	y = max((int)((size.height-height)/2),0);
	for( i = 0;i<(int)parent->getChildCount();++i ){
		int col,row;
		MyGUI::Widget* pchild = parent->getChildAt(i);
		//这里使用中心对齐
		col = i%column;
		row = (int)(i/column);
		pchild->setPosition(MyGUI::IntPoint(x+col*space+sum(mv,col),y+sum(mh,row)));
		pchild->setSize(MyGUI::IntSize(mv[col],mh[row]));
	}
	//如果父窗口是ScrollView
	
	if( psw )psw->setCanvasSize( max(size.width,width),max(size.height,height) );
}

/*改变SimpleData数据，然后调用通知函数，报告已经改变。
*/
static void _simpleDataChange(MyGUI::ComboBox* _sender, size_t _index){
	SimpleData* psd = _sender->getUserData<SimpleData>(false);
	if( psd ){
		MyGUI::UString s = _sender->getItemNameAt(_index);
		if( psd->type==SimpleData::STRING )
			psd->str = s;
		else if( psd->type==SimpleData::REAL ){
			try{
				psd->real = boost::lexical_cast<Ogre::Real>(s);
			}catch(bad_cast& e ){
				MYGUI_LOG(Warning,e.what());
			}
		}
		if( psd->change )
			psd->change->invoke(psd);
	}
}

/*SimpleData数据改变
*/
static void _simpleDataEditTextChange(MyGUI::EditBox* _sender){
	SimpleData* psd = _sender->getUserData<SimpleData>(false);
	if( psd ){
		if( psd->type==SimpleData::STRING )
			psd->str = _sender->getCaption();
		else if( psd->type==SimpleData::REAL ){
			try{
				psd->real = boost::lexical_cast<Ogre::Real>(_sender->getCaption());
			}catch(bad_cast& e ){
				MYGUI_LOG(Warning,e.what());
			}
		}
		if( psd->change )
			psd->change->invoke(psd);
	}
}

/*BOOL型数据改变 Check
*/
static void _simpleDataCheckChange(MyGUI::Widget* _sender){
	MyGUI::Button* sender = _sender->castType<MyGUI::Button>(false);
	if( sender ){
		SimpleData* psd = sender->getUserData<SimpleData>(false);
		sender->setStateSelected(!sender->getStateSelected());
		if( psd ){
			if( psd->type==SimpleData::BOOL ){
				psd->b = sender->getStateSelected();
			}
			if( psd->change )
				psd->change->invoke(psd);
		}
	}
}

/*向界面中加入数据相
*/
void SimpleDataUI::add( const MyGUI::UString& caption,SimpleData sd ){

	MyGUI::TextBox* pt = mParent->createWidget<MyGUI::TextBox>(
		"TextBox",MyGUI::IntCoord(),
		MyGUI::Align::Left|MyGUI::Align::Top);
	
	pt->setTextAlign( MyGUI::Align::Right );
	pt->setCaption( caption );
	pt->setUserData( MyGUI::Any(string("@")) ); //打一个标记为删除做准备

	if( sd.type==SimpleData::BOOL ){
		MyGUI::Button* pe = mParent->createWidget<MyGUI::Button>(
		"CheckBox",MyGUI::IntCoord(),
		MyGUI::Align::Left|MyGUI::Align::Top);
		pe->setStateSelected(sd.b);
		sd.change = mep;
		pe->setUserData(MyGUI::Any(sd));
		pe->eventMouseButtonClick += newDelegate(_simpleDataCheckChange);
	}else if( sd.sv.empty() ){//编辑
		MyGUI::EditBox* pe = mParent->createWidget<MyGUI::EditBox>(
		"EditBox",MyGUI::IntCoord(),
		MyGUI::Align::Left|MyGUI::Align::Top);
		if( sd.type== SimpleData::STRING )
			pe->setCaption( sd.str );
		else if( sd.type== SimpleData::REAL)
		{
			pe->setCaption( (boost::format("%.2f")%sd.real).str() );
		}

		sd.change = mep;
		pe->setUserData(MyGUI::Any(sd));
		//数据改变
		pe->eventEditTextChange += newDelegate(_simpleDataEditTextChange);
	}else{//有可选数据
		MyGUI::ComboBox* pc = mParent->createWidget<MyGUI::ComboBox>(
		"ComboBox",MyGUI::IntCoord(),
		MyGUI::Align::Left|MyGUI::Align::Top);
		if( sd.type== SimpleData::STRING )
			pc->setCaption( sd.str );
		else if( sd.type== SimpleData::REAL )
			pc->setCaption( boost::lexical_cast<string>(sd.real) );
		for( vector<MyGUI::UString>::const_iterator i = sd.sv.begin();
			i!=sd.sv.end();++i){
			pc->addItem(*i);
			if( *i == sd.str ){
				if( sd.type== SimpleData::STRING )
					pc->setEditStatic(true);
				pc->setIndexSelected(i-sd.sv.begin());
			}
		}

		sd.change = mep;
		pc->setUserData(MyGUI::Any(sd));
		//数据改变
		pc->eventComboChangePosition += newDelegate(_simpleDataChange);
	}
}

SimpleData SimpleDataUI::get( string name ){
	assert( mParent );
	SimpleData sd;

	for( size_t i = 0;i<mParent->getChildCount();++i ){
		MyGUI::Widget* pw = mParent->getChildAt(i); 
		SimpleData* psd = pw->getUserData<SimpleData>(false);
		if( psd && psd->name==name ){
			MyGUI::UString us;
			MyGUI::EditBox* pe = pw->castType<MyGUI::EditBox>(false);
			if( pe ){
				us = pe->getCaption();
				if( psd->type==SimpleData::STRING )
					psd->str = us;
				else if( psd->type==SimpleData::REAL ){
					try{
						psd->real = boost::lexical_cast<Ogre::Real>( us );
					}catch( boost::bad_lexical_cast& e ){
						MYGUI_LOG(Warning,e.what());
					}
				}
			}
			return *psd;
		}
	}
	return sd;
}

/*清除通过Add加入的界面物件
*/
void SimpleDataUI::clear(){
	assert( mParent );
	MyGUI::VectorWidgetPtr dv;
	for( size_t i = 0;i<mParent->getChildCount();++i ){
		MyGUI::Widget* pw = mParent->getChildAt(i); 
		SimpleData* psd = pw->getUserData<SimpleData>(false);
		if( psd )dv.push_back(pw);

		string* ps = pw->getUserData<string>(false);
		if( ps && *ps == "@" )dv.push_back(pw);
	}
	MyGUI::Gui::getInstancePtr()->destroyWidgets( dv );
}

void SimpleUI::checkItem( const string& name,bool b )
{
	try{
		MyGUI::Button * c = operator [](name)->castType<MyGUI::Button>(false);
		if( c )
			c->setStateSelected( b );
	}catch( out_of_range& e ){
		WARNING_LOG(e.what());
	}
}

void SimpleUI::addItem( const string& name,const string& tex )
{
	try
	{
		MyGUI::ComboBox * c = operator [](name)->castType<MyGUI::ComboBox>(false);
		if( c )
			c->addItem( tex );
		else
		{
			MyGUI::ListBox * c = operator [](name)->castType<MyGUI::ListBox>(false);
			if( c )
				c->addItem( tex );
		}
	}
	catch( std::out_of_range& e )
	{
		WARNING_LOG(e.what());
	}
}

void SimpleUI::addItem( const string& name,const string* tex,int count )
{
	for( int i=0;i<count;++i )
		addItem( name,tex[i] );
}

void SimpleUI::selectItem( const string& name,int index )
{
	try
	{
		MyGUI::ComboBox * c = operator [](name)->castType<MyGUI::ComboBox>(false);
		if( c )
			c->setIndexSelected( index );
		else
		{
			MyGUI::ListBox * c = operator [](name)->castType<MyGUI::ListBox>(false);
			if( c )
				c->setIndexSelected( index );
		}
	}
	catch( std::out_of_range& e )
	{
		WARNING_LOG(e.what());
	}
}

void SimpleUI::removeAllItem( const string& name )
{
	try
	{
		MyGUI::ComboBox * c = operator [](name)->castType<MyGUI::ComboBox>(false);
		if( c )
			c->removeAllItems();
		else
		{
			MyGUI::ListBox * c = operator [](name)->castType<MyGUI::ListBox>(false);
			if( c )
				c->removeAllItems();
		}
	}
	catch( std::out_of_range& e )
	{
		WARNING_LOG(e.what());
	}
}