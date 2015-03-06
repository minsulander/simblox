#ifndef SBX_FACTORY_H
#define SBX_FACTORY_H

#include <vector>
#include <map>

namespace sbx {
	
	template<class Tbase>
	class CreatorBase {
	public:
		virtual ~CreatorBase() {}
		virtual Tbase * create() const = 0;
	};
	
	template<class Product, class Tbase>
	class Creator : public CreatorBase<Tbase> {
	public:
		virtual Tbase * create() const 
		{ 
			Product* obj = new Product;
			return static_cast<Tbase*>(obj);
		}
	};
	
	template<typename Tid, class Tbase>
	class Factory {
	public:
		bool registerObject(Tid type, CreatorBase<Tbase> * pCreator);
		bool hasRegistered(Tid type) { return (creatormap.find(type) != creatormap.end()); }
		Tbase * create(Tid type);
		typedef std::vector<Tid> IdentifierList;
		IdentifierList getIdentifierList();
	private:
		typedef std::map<Tid, CreatorBase<Tbase> *> CreatorMap;
		CreatorMap creatormap;
	};
	
	template<typename Tid, class Tbase>
	bool Factory<Tid, Tbase>::registerObject(Tid type, CreatorBase<Tbase> * pCreator)
	{
		typename CreatorMap::iterator it = creatormap.find(type);
		if (it != creatormap.end()) {
			delete pCreator;
			return false;
		}
		creatormap[type] = pCreator;
		return true;
	}
	
	template<typename Tid, class Tbase>
	Tbase * Factory<Tid, Tbase>::create(Tid type)
	{
		typename CreatorMap::iterator it = creatormap.find(type);
		if (it == creatormap.end()) 
			return NULL;
		
		CreatorBase<Tbase> * pCreator = (*it).second;
		return pCreator->create();
	}
	
	template<typename Tid, class Tbase>
	std::vector<Tid> Factory<Tid, Tbase>::getIdentifierList()
	{
		std::vector<Tid> vec;
		for (typename CreatorMap::iterator i = creatormap.begin(); i != creatormap.end(); i++)
			vec.push_back(i->first);
		return vec;
	}
	
} // namespace

#endif
