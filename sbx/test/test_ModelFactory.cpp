#include <UnitTest++/UnitTest++.h>
#include <sbx/ModelFactory.h>
#include <smrt/ref_ptr.h>

using namespace sbx;

TEST(ModelFactory) {
	
	std::vector<std::string> objects = ModelFactory::instance().getIdentifierList();
	CHECK(objects.size() > 0);
	
	// Create a model from the factory
	smrt::ref_ptr<Model> model = ModelFactory::instance().create(objects[0]);
	CHECK(model.valid());
	
	// Create object using full name
	std::string fullname = model->libraryName() + std::string("::") + model->className();
	model = ModelFactory::instance().create(fullname);
	CHECK(model.valid());
}
