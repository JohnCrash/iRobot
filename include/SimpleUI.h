#ifndef _SIMPLEUI_H_
#define _SIMPLEUI_H_
/* SimpleUI简化界面编程
	用法：定义一个SimpleUI mUI;
	然后可以使用mUI.load加载.layout文件
	然后使用mUI["WidgetName"]->访问部件，使用mUI.close摧毁窗口
	mUI.IsWidget简单的加入前缀，用在事件函数中
*/

//计算一个字串的长度
MyGUI::IntSize CalcTextWidth( MyGUI::UString text,string font );
//计算一个物件的大小，根据文字
MyGUI::IntSize CalcWidgetSize(MyGUI::Widget* pw);

class SimpleUI
{
public:
	SimpleUI(){}

	void load( const string& layout,const string& prefix ){
		mLayout = layout;
		mPrefix = prefix;
		mVw = MyGUI::LayoutManager::getInstancePtr()->loadLayout(layout,prefix);
		MyGUI::IntSize size = MyGUI::RenderManager::getInstance().getViewSize();
		MyGUI::IntSize newSize = size;
		//启用默认对齐方式
		for_each( mVw.begin(),mVw.end(),boost::bind(&MyGUI::Widget::_setAlign,_1,size,newSize) );
		//mVw[0]->_setAlign( size,newSize );
	}

	void close(){
		MyGUI::LayoutManager::getInstancePtr()->unloadLayout(mVw);
		mVw.clear();
	}

	virtual ~SimpleUI(){}

	MyGUI::Widget* operator []( const string& widget ){
		MyGUI::Widget* pwig = MyGUI::Gui::getInstancePtr()->findWidgetT(widget,mPrefix,false);
		if( !pwig )
			throw out_of_range( mLayout +"["+ widget +"] not found.");
		return pwig;
	}

	MyGUI::Window* operator->(){
		assert(mVw[0]);
		return mVw[0]->castType<MyGUI::Window>(false);
	}

	bool IsWidget(MyGUI::Widget* _widget,const string _name ){
		return (_widget->getName()==mPrefix+_name);
	}

	operator void* (){
		return (void*)(mVw.empty()?0:1);
	}

	bool operator ! (){
		return mVw.empty();
	}

	/*一些简化函数
	checkItem操作CheckBox
	setCaption设置标题，EditBox可以设置内容
	addItem加入列表相，ComboBox
	selectItem选择ComboBox项目
	*/
	void checkItem( const string& name,bool b );
	template <typename A> void setCaption( const string& name,const string& text )
	{
		try{
			A * c = operator [](name)->castType<A>(false);
			if( c )
				c->setCaption( text );
		}catch( out_of_range& e ){
			WARNING_LOG(e.what());
		}
	}
	void addItem( const string& name,const string& tex );
	void addItem( const string& name,const string* tex,int count );
	void selectItem( const string& name,int index );
	void removeAllItem( const string& name );
protected:
	MyGUI::VectorWidgetPtr mVw;
	string mPrefix;
	string mLayout;
};

/*对子窗口进行排列布局
*/
class Grid{
public:
	Grid( MyGUI::Widget* p,int _col,int _space ):
	  parent(p),column(_col),space(_space){
	}

	void reLayout();
protected:
	int sum(vector<int>& v,int c){
		int s = 0;
		for( int i=0;i<min((int)v.size(),c);++i )
			s += v[i];
		return s;
	}

	MyGUI::Widget* parent;
	int column;
	int space;//行之间的间隔
};

/*创建一个简单的界面用来交换数据
*/

class SimpleData;

typedef MyGUI::delegates::CMultiDelegate1<SimpleData*> EventHandle_SimpleDataChange;
typedef EventHandle_SimpleDataChange::IDelegate  SimpleDataChangeIDelegate;

class SimpleData{
public:
	enum Type{
		STRING,
		REAL,
		BOOL,
		UNKNOW
	};

	//默认构造
	SimpleData():type(UNKNOW){
	}

	//复制构造
	SimpleData( const SimpleData& sd ):
	type(sd.type),name(sd.name),
	str(sd.str),real(sd.real),b(sd.b),
	sv(sd.sv),change(sd.change){
	}

	SimpleData& operator = ( const SimpleData& sd ){
		type = sd.type;
		name = sd.name;
		str = sd.str;
		real =sd.real;
		b = sd.b;
		sv = sd.sv;
		change = sd.change;
		return *this;
	}

	SimpleData( const string& _n,const MyGUI::UString& _s ):
		type(STRING),name(_n),str(_s),real(0),b(false),sv(),
		change((SimpleDataChangeIDelegate*)nullptr){
	}
	
	SimpleData( const string& _n,const MyGUI::UString& _s,
		const vector<MyGUI::UString>& _sv ):
		type(STRING),name(_n),str(_s),real(0),b(false),sv(_sv),
		change((SimpleDataChangeIDelegate*)nullptr){
	}
	
	SimpleData( const string& _n,double r):
		type(REAL),name(_n),str(""),real(r),b(false),sv(),
		change((SimpleDataChangeIDelegate*)nullptr){
	}
	
	SimpleData( const string& _n,double r,
		const vector<MyGUI::UString>& _sv):
		type(REAL),name(_n),str(""),real(r),b(false),sv(_sv),
		change((SimpleDataChangeIDelegate*)nullptr){
	}

	SimpleData( const string& _n,bool _b):
		type(BOOL),name(_n),str(""),real(0),b(_b),sv(),
		change((SimpleDataChangeIDelegate*)nullptr){
	}

	SimpleData( const string& _n,bool _b,
		const vector<MyGUI::UString>& _sv):
		type(BOOL),name(_n),str(""),real(0),b(_b),sv(_sv),
		change((SimpleDataChangeIDelegate*)nullptr){
	}

	Type type;
	string	name; //变量名称
	MyGUI::UString str; //显示内容
	double		real;
	bool			b;
	vector<MyGUI::UString> sv; //可选的值
	boost::shared_ptr<SimpleDataChangeIDelegate> change;
};


/*在父窗口中创建对应的相
*/
class SimpleDataUI{
public:
	SimpleDataUI( MyGUI::Widget* _parent ):
	  mm(),mParent(_parent),mY(0),mep((SimpleDataChangeIDelegate*)nullptr){
	}

  	SimpleDataUI( MyGUI::Widget* _parent,SimpleDataChangeIDelegate* p ):
	  mm(),mParent(_parent),mY(0),mep(p){
	}

	void add( const MyGUI::UString& description,SimpleData sd );

	void reLayout( int col,int space ){
		Grid grid(mParent,col,space );
		grid.reLayout();
	}

	SimpleData get( string name );

	void clear();

	virtual ~SimpleDataUI(){
	}
protected:
	map<string,SimpleData> mm;
	MyGUI::Widget* mParent;
	int mY;
	boost::shared_ptr<SimpleDataChangeIDelegate> mep;
};
#endif