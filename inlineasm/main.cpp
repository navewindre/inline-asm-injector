#include <Windows.h>
#include <stdio.h>

const char* csgo_str = "Counter-Strike: Global Offensive";
const char* hack_str = "hack.dll";
const char* loadlib_str = "LoadLibraryA";
const char* k32_str = "kernel32.dll";
const char* fail_str = "injection failed\n";

__declspec( naked ) int get_csgo_process( ) {
	__asm {
		push ebp
		mov ebp, esp
		sub esp, 8

		mov eax, csgo_str
		push eax
		push 0
		call ds:FindWindowA

		mov [ ebp - 4 ], eax

		cmp [ ebp - 4 ], 0
		je FAIL
		
		lea eax, [ ebp - 8 ]
		push eax
		mov ecx, [ ebp - 4 ]
		push ecx
		call GetWindowThreadProcessId

		cmp eax, 0
		je FAIL

		jmp END

		FAIL:
			push fail_str
			call printf
			mov eax, 0

		END:
			mov eax, [ ebp - 8 ]
			mov esp, ebp
			pop ebp
			ret
	}
}

__declspec( naked ) HANDLE open_process_handle( int pid ) {
	__asm {
		push ebp
		mov ebp, esp

		mov eax, pid
		mov ecx, PROCESS_ALL_ACCESS

		push eax
		push 0
		push ecx
		call ds:OpenProcess

		cmp eax, 0
		je FAIL

		jmp END

		FAIL:
			push fail_str
			call printf
			mov eax, 0

		END:
			mov esp, ebp
			pop ebp
			ret
	}
}

__declspec( naked ) uintptr_t get_loadlib_addr( ) {
	__asm {
		push ebp
		mov ebp, esp

		push k32_str
		call ds:GetModuleHandleA

		push loadlib_str
		push eax
		call ds:GetProcAddress

		mov esp, ebp
		pop ebp
		ret
	}
}

__declspec( naked ) bool inject( HANDLE proc ) {
	__asm {
		push ebp
		mov ebp, esp
		sub esp, 16

		call get_loadlib_addr
		mov [ ebp - 4 ], eax

		mov ecx, PAGE_EXECUTE_READWRITE
		push ecx
		mov eax, ( MEM_RESERVE | MEM_COMMIT )
		push eax
		mov edx, 9
		push edx
		push 0
		push proc
		call ds:VirtualAllocEx

		mov [ ebp - 8 ], eax
		
		push 0
		mov edx, 9
		push edx
		mov eax, hack_str
		push eax
		mov ecx, [ ebp - 8 ]
		push ecx
		push proc
		call ds:WriteProcessMemory

		push 0
		push 0
		mov eax, [ ebp - 8 ]
		push eax
		mov edx, [ ebp - 4 ]
		push edx
		push 0
		push 0
		push proc
		call ds:CreateRemoteThread

		mov [ ebp - 12 ], eax

		mov edx, 0xffffffff
		push edx
		mov eax, [ ebp - 12 ]
		push eax
		call ds:WaitForSingleObject

		mov esp, ebp
		pop ebp
		ret
	}
}

__declspec( naked ) int main( ) {
	__asm {
		push ebp
		mov ebp, esp

		call get_csgo_process
		push eax
		call open_process_handle
		push eax
		call inject

		mov esp, ebp
		pop ebp
		ret
	}

//	auto pid = get_csgo_process( );
//	auto handle = open_process_handle( pid );
//	inject( handle );
}