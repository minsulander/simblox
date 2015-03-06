#include "OperatorModels.h"
#include "ModelFactory.h"

namespace op {
	
	REGISTER_Object(op, Constant);
	REGISTER_Object(op, Add);
	REGISTER_Object(op, Subtract);
	REGISTER_Object(op, Multiply);
	REGISTER_Object(op, Divide);
	
	void Constant::init() { out = value; }

	void Add::init() { c = *a + *b; }
	void Add::update(const double dt) { c = *a + *b; }

	void Subtract::init() { c = *a - *b; }
	void Subtract::update(const double dt) { c = *a - *b; }
	
	void Multiply::init() { c = *a * *b; }
	void Multiply::update(const double dt) { c = *a * *b; }
	
	void Divide::init() { c = *a / *b; }
	void Divide::update(const double dt) { c = *a / *b; }
	
} // namespace
