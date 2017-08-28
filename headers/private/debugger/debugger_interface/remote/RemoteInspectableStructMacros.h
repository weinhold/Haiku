/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */


#define DECLARE_REQUEST_AND_RESPONSE_STRUCT(name, requestFields,	\
		responseFields)												\
	struct name ## Request;											\
	struct name ## Response;

#define DECLARE_VISITOR(name, type, ...)									\
	struct name {															\
		virtual ~name() {}													\
		ITERATE3(DECLARE_VISIT_METHOD_ ## type, DEFINE_EMPTY, __VA_ARGS__)	\
	};

#define DECLARE_VISIT_METHOD_Request(name, requestFields, responseFields)		\
	virtual void Visit(name ## Request* request) = 0;
#define DECLARE_VISIT_METHOD_Response(name, requestFields, responseFields)		\
	virtual void Visit(name ## Response* response) = 0;

#define DEFINE_REQUEST_AND_RESPONSE_STRUCT(name, requestFields,		\
		responseFields)												\
	struct name ## Response;										\
	DEFINE_REQUEST_STRUCT(name, UNWRAP_MACRO_ARGS requestFields)	\
	DEFINE_REPLY_STRUCT(name, UNWRAP_MACRO_ARGS responseFields)		\
	template<>														\
	struct RemoteResponse<name ## Request> {						\
		typedef name ## Response Type;								\
	};
