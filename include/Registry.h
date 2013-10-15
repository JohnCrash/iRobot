#ifndef _REGISTRY_H_
#define _REGISTRY_H_
/*
	提供一个可以保存到磁盘的数据管理功能
*/
class Registry:public Ogre::Singleton<Registry>
{

protected:
	void load(const string& file);
	void save(const string& file);
	string mFileName;

	typedef map<string,string> RegMap;
	RegMap mRegs;
public:
	Registry();
	~Registry();

	string get(const string& key);

	template <typename T> T get(const string& key,T def ) const
	{
		RegMap::const_iterator it = mRegs.find(key);
		if( it == mRegs.end() )
		{
			return def;
		}
		else
		{
			T r;
			try
			{
				r = boost::lexical_cast<T>(it->second);
			}
			catch(boost::bad_lexical_cast& e)
			{
				WARNING_LOG("Registry get key "<<e.what());
				return def;
			}
			return r;
		}
	}

	void set(const string& key,const string& value);

	template <typename T> void set( const string& key,T value )
	{
		try
		{
			mRegs[key] = boost::lexical_cast<string>(value);
		}
		catch(boost::bad_lexical_cast& e)
		{
			WARNING_LOG("Registry set key "<<e.what());
		}
	}
};

#endif