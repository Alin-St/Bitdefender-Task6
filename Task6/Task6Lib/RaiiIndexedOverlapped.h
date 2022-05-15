#pragma once

#include <Windows.h>

/// <summary> Similar to OVERLAPPED, but when the object is destroyed or assigned it closes the underlying event handle if it's not 0.
/// Also has an index extension and it's initialized to 0 by default. Copy constructor and assignment are not provided. </summary>
class RaiiIndexedOverlapped
{
public:
	// Use a char array instead of separated members to have the memory layout guarantee, without POD class.
	char ptrOverlapped[sizeof(OVERLAPPED) + sizeof(size_t)] = { 0 };

public:
	/// <summary> Initialize the OVERLAPPED structure and index with 0 (including the event handle). </summary>
	RaiiIndexedOverlapped() noexcept = default;

	/// <summary> Initialize the OVERLAPPED structure with 0, and optionally create an event for it. </summary>
	/// <param name="withEvent"> true if you want to create a manual reset event for the OVERLAPPED structure, false to initialize the event to 0. </param>
	RaiiIndexedOverlapped(bool withEvent, size_t index = 0);

	RaiiIndexedOverlapped(const RaiiIndexedOverlapped&) = delete;
	RaiiIndexedOverlapped(RaiiIndexedOverlapped&&) = delete;
	RaiiIndexedOverlapped& operator=(const RaiiIndexedOverlapped&) = delete;
	RaiiIndexedOverlapped& operator=(RaiiIndexedOverlapped&&) = delete;

	~RaiiIndexedOverlapped() noexcept;

	operator OVERLAPPED& () noexcept;

	/// <summary> Get a reference to the index of a RaiiIndexedOverlapped structure. If you only have a pointer to the OVERLAPPED structure consider using
	/// RaiiIndexedOverlapped::indexFromOverlappedLp function. </summary>
	size_t& index() noexcept;

	/// <summary> If you have a pointer to an OVERLAPPED structure that belongs to a RaiiIndexedOverlapped, you can use this function to get a reference
	/// to the index associated with that OVERLAPPED. </summary>
	static size_t& indexFromOverlappedLp(OVERLAPPED* lpOverlapped) noexcept;
};
