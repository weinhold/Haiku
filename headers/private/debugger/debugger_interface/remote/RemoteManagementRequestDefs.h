/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */


// The including file must define DEFINE_REQUEST_AND_RESPONSE_STRUCTS() as
// needed. The header can be included multiple times for different purposes.


DEFINE_REQUEST_AND_RESPONSE_STRUCTS(
	Hello,
	(
		uint32,		protocolVersion
	),
	(
		status_t, 	error,
		uint32,		protocolVersion
	),

	GetTeams,
	(
	),
	(
		status_t, 				error,
		BObjectList<TeamInfo>,	infos
	),

	AttachToTeam,
	(
		int32,		teamId,
		int32,		threadId
	),
	(
		status_t, 	error,
		uint64,		channel
	)

)
