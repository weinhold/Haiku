/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef UTIL_REFERENCE_H
#define UTIL_REFERENCE_H


#include <Referenceable.h>


/*!	Object pointer wrapper that can optionally own the referenced object,
	respectively -- if the object is a BReferenceable -- a reference to it.
*/
template<typename Value>
struct Reference {
								Reference();
								Reference(const Value* value);
								Reference(Value* value, bool ownsValue);
								Reference(const Reference& other);
								~Reference();

			Reference&			operator=(const Reference& other);

			void				SetTo(const Value* value);
			void				SetTo(Value* value, bool ownsValue);
			void				Unset();

			const Value*		Get() const		{ return fValue; }
			Value*				Get()			{ return fValue; }
			Value*				Detach();

			const Value&		operator*() const	{ return *fValue; }
			const Value*		operator->() const	{ return fValue; }

private:
			// helper functions for releasing our fValue reference
			void				_ReleaseValueReference(void*)
									{ delete fValue; }
			void				_ReleaseValueReference(BReferenceable*)
									{ fValue->ReleaseReference(); }

private:
			Value*				fValue;
			bool				fOwnsValue;
};


template<typename Value>
inline
Reference<Value>::Reference()
	:
	fValue(NULL),
	fOwnsValue(false)
{
}


template<typename Value>
inline
Reference<Value>::Reference(const Value* value)
	:
	fValue(const_cast<Value*>(value)),
	fOwnsValue(false)
{
}


template<typename Value>
inline
Reference<Value>::Reference(Value* value, bool ownsValue)
	:
	fValue(value),
	fOwnsValue(ownsValue)
{
}


template<typename Value>
inline
Reference<Value>::Reference(const Reference& other)
	:
	fValue(other.fValue),
	fOwnsValue(false)
{
}


template<typename Value>
inline
Reference<Value>::~Reference()
{
	Unset();
}


template<typename Value>
inline Reference<Value>&
Reference<Value>::operator=(const Reference& other)
{
	SetTo(other.fValue, false);
	return *this;
}


template<typename Value>
inline void
Reference<Value>::SetTo(const Value* value)
{
	SetTo(const_cast<Value*>(value), false);
}


template<typename Value>
inline void
Reference<Value>::SetTo(Value* value, bool ownsValue)
{
	Unset();

	fValue = value;
	fOwnsValue = ownsValue;
}


template<typename Value>
inline void
Reference<Value>::Unset()
{
	if (fOwnsValue)
		_ReleaseValueReference(fValue);

	fValue = NULL;
	fOwnsValue = false;
}


template<typename Value>
inline Value*
Reference<Value>::Detach()
{
	Value* value = fOwnsValue ? fValue : NULL;
	fValue = NULL;
	fOwnsValue = false;
	return value;
}


#endif	// UTIL_REFERENCE_H
