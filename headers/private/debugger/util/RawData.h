/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef UTIL_RAW_DATA_H
#define UTIL_RAW_DATA_H


#include <stdlib.h>

#include <stdexcept>


struct RawData {
			enum OwnershipPolicy {
				Reference,
				AsOriginal,
				Clone
			};
public:
								RawData();
			// Note: may throw std::bad_alloc, if cloning required
								RawData(const void* data, size_t size,
									OwnershipPolicy ownershipPolicy);
								RawData(const RawData& other,
									OwnershipPolicy ownershipPolicy
										= AsOriginal);
								~RawData();

			RawData&			operator=(const RawData& other);

			// Note: may throw std::bad_alloc, if cloning required
			void				SetTo(const RawData& other,
									OwnershipPolicy ownershipPolicy);
			void				SetTo(const void* data, size_t size,
									OwnershipPolicy ownershipPolicy);
			void				Unset();

			const void*			Data() const	{ return fData; }
			size_t				Size() const	{ return fSize; }

private:
			void*				fData;
			size_t				fSize;
			bool				fOwnsData;
};


inline
RawData::RawData()
	:
	fData(NULL),
	fSize(0),
	fOwnsData(false)
{
}


inline
RawData::RawData(const void* data, size_t size, OwnershipPolicy ownershipPolicy)
	:
	fData(NULL),
	fSize(0),
	fOwnsData(false)
{
	SetTo(data, size, ownershipPolicy);
}


inline
RawData::RawData(const RawData& other, OwnershipPolicy ownershipPolicy)
	:
	fData(NULL),
	fSize(0),
	fOwnsData(false)
{
	SetTo(other, ownershipPolicy);
}


inline
RawData::~RawData()
{
	Unset();
}


inline RawData&
RawData::operator=(const RawData& other)
{
	SetTo(other, AsOriginal);
	return *this;
}


inline void
RawData::SetTo(const RawData& other, OwnershipPolicy ownershipPolicy)
{
	if (ownershipPolicy == AsOriginal)
		ownershipPolicy = other.fOwnsData ? Clone : Reference;
	SetTo(other.fData, other.fSize, ownershipPolicy);
}


inline void
RawData::SetTo(const void* data, size_t size, OwnershipPolicy ownershipPolicy)
{
	Unset();

	if (ownershipPolicy == Clone && data != NULL) {
		fData = malloc(size);
		if (fData == NULL)
			throw std::bad_alloc();
		fSize = size;
		fOwnsData = true;
	} else {
		fData = const_cast<void*>(data);
		fSize = size;
		fOwnsData = false;
	}
}


inline void
RawData::Unset()
{
	if (fOwnsData)
		free(fData);

	fData = NULL;
	fSize = 0;
	fOwnsData = false;
}


#endif	// UTIL_RAW_DATA_H
