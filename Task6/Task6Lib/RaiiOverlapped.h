#pragma once

#include <Windows.h>

/// <summary> Similar to OVERLAPPED, but when the object is destroyed or assigned it closes the underlying event handle if it's not 0.
/// Copy or move constructor and assignment are not provided. </summary>
class RaiiOverlapped
{
public:
	OVERLAPPED overlapped = { 0 };

public:
	/// <summary> Initialize the OVERLAPPED structure with 0 (including the event handle). </summary>
	RaiiOverlapped() noexcept = default;

	/// <summary> Initialize the OVERLAPPED structure with 0, and optionally create an event for it. </summary>
	/// <param name="withEvent"> true if you want to create a manual reset event for the OVERLAPPED structure, false to initialize the event to 0. </param>
	RaiiOverlapped(bool withEvent);

	RaiiOverlapped(const RaiiOverlapped&) = delete;
	RaiiOverlapped(RaiiOverlapped&&) = delete;
	RaiiOverlapped& operator=(const RaiiOverlapped&) = delete;
	RaiiOverlapped& operator=(RaiiOverlapped&&) = delete;

	~RaiiOverlapped() noexcept;

	operator OVERLAPPED& () noexcept;
};
