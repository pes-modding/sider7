--[[ codeinject.lua

Code injection example:
1. Allocates new memory
2. Writes code fragment to it
3. Put a call instruction in existing code to call our newly written code

--]]

local m = {}
local hex = memory.hex

function m.init(ctx)
    -- step 1: allocate memory.
    local addr = memory.allocate_codecave(512)
    log(string.format("memory allocated at: %s", hex(addr)))

    -- step 2: write code fragment into newly allocated area:
    -- we will use a simple function as an example:
    --
    -- 00000000067F0000 | 48:B8 EFCDAB8967452301          | mov rax,123456789ABCDEF
    -- 00000000067F000A | C3                              | ret
    local code = "\x48\xb8\xef\xcd\xab\x89\x67\x45\x23\x01\xc3"
    memory.write(addr, code)
    log(string.format("code written at: %s, size: %d", hex(addr), #code))

    -- step 3: plant a call instruction from a known spot in PES.
    -- We will search for this code fragment:
    --
    -- 0000000140B17478 | 0FB647 4A                       | movzx eax,byte ptr ds:[rdi+4A]          | rdi+4A:"ARS"
    -- 0000000140B1747C | 48:8D4B 50                      | lea rcx,qword ptr ds:[rbx+50]           |
    -- 0000000140B17480 | 8843 4A                         | mov byte ptr ds:[rbx+4A],al             |
    -- 0000000140B17483 | BA B5000000                     | mov edx,B5                              |
    local pes_code_fragment = "\x0f\xb6\x47\x4a\x48\x8d\x4b\x50\x88\x43\x4a\xba\xb5\x00\x00\x00"
    local call_site = memory.search_process(pes_code_fragment)
    if not call_site then
        error("unable to find known code pattern in PES exe")
    end
    log(string.format("pes code fragment found at: %s", hex(call_site)))

    -- now we need to replace it with a call-out to our function.
    -- But if we do just that, then we lose the original logic that we replaced with our call
    -- So we need to preserve it and execute that logic after our function is done.
    -- 
    -- In order to accomplish that, we create another helper-function, which first calls our simple
    -- function, but then executes the replaced PES code. We place that helper-function, right after
    -- the one we created earlier:
    --
    -- 00000000067F000B | 48:B8 00007F0600000000          | mov rax,67F0000                         |
    -- 00000000067F0015 | FFD0                            | call rax                                |
    -- 00000000067F0017 | 0FB647 4A                       | movzx eax,byte ptr ds:[rdi+4A]          | rdi+4A:"ARS"
    -- 00000000067F001B | 48:8D4B 50                      | lea rcx,qword ptr ds:[rbx+50]           |
    -- 00000000067F001F | 8843 4A                         | mov byte ptr ds:[rbx+4A],al             |
    -- 00000000067F0022 | BA B5000000                     | mov edx,B5                              |
    -- 00000000067F0027 | C3                              | ret                                     |
    local addr2 = addr + #code
    memory.write(addr2,
        "\x48\xb8" .. memory.pack("u64", addr) .. "\xff\xd0" ..  -- call to our simple function
        pes_code_fragment .. -- replaced code from PES
        "\xc3") -- return instruction

    -- now we patch PES to call to our helper-function
    memory.write(call_site,
        "\x48\xb8" .. memory.pack("u64", addr2) .. "\xff\xd0" ..  -- call to our helper function
        "\x90\x90\x90\x90" -- nop the remaining 4 bytes of the original code
    )

    log(string.format("you can verify the call by attaching a debugger to PES exe"
        .. " and setting up breakpoint at %s or at %s", hex(addr), hex(call_site)))
    log("The break should happen several times, after 2 teams are selected for a match")
end

return m
