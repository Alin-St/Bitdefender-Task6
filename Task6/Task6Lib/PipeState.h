#pragma once

/// <summary> A pipe can be either unconnected, connected or disconnecting. An unconnected pipe can be connecting, a connected pipe
/// can be reading and / or writing, and a disconnecting pipe can be connecting or (reading and / or writing). </summary>
enum class PipeState : int
{
	// PIPE TYPE

	/// <summary> Pipe is not connected. An unconnected pipe can also be connecting (if bit operations are used). </summary>
	UNCONNECTED = 0x000,

	/// <summary> Pipe is connected. A connected pipe can also be reading and / or writing (if bit operations are used). </summary>
	CONNECTED = 0x001,

	/// <summary> Pipe is disconnecting. A disconnecting pipe can also be connecting or (reading and / or writing). </summary>
	DISCONNECTING = 0x002,

	// ASYNC STATE
	
	/// <summary> The unconnected or disconnecting pipe is connecting. This flag should be used only with bit operators. </summary>
	FLAG_CONNECTING = 0x010,

	/// <summary> The connected or disconnecting pipe is reading. This flag should be used only with bit operators. </summary>
	FLAG_READING = 0x020,

	/// <summary> The connected or disconnecting pipe is writing. This flag should be used only with bit operators. </summary>
	FLAG_WRITING = 0x040,

	// HELPER VALUES

	_UNCONNECTED_CONNECTING = UNCONNECTED | FLAG_CONNECTING,
	_CONNECTED_READING = CONNECTED | FLAG_READING,
	_CONNECTED_WRITING = CONNECTED | FLAG_WRITING,
	_CONNECTED_READING_WRITING = CONNECTED | FLAG_READING | FLAG_WRITING,
	_DISCONNECTING_CONNECTING = DISCONNECTING | FLAG_CONNECTING,
	_DISCONNECTING_READING = DISCONNECTING | FLAG_READING,
	_DISCONNECTING_WRITING = DISCONNECTING | FLAG_WRITING,
	_DISCONNECTING_READING_WRITING = DISCONNECTING | FLAG_READING | FLAG_WRITING,
};
