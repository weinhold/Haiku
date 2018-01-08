/*
 * Copyright 2017, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */


// The including file must define DEFINE_EVENT_STRUCTS() as needed. The header
// can be included multiple times for different purposes.


DEFINE_EVENT_STRUCTS(
	TeamAdded,
	(
		TeamInfo,	info
	),

	TeamRemoved,
	(
		int32,		teamId
	),

	TeamRenamed,
	(
		TeamInfo,	info
	)
)
