#!c:\python\python.exe

"""
Proof-of-Concept In Memory Fuzzer from Chapter 19/20

Note:
    The code in the book looks a bit different then the code here because the interface to function callbacks in PyDbg
    has since been simplified.
"""

import time
import random
import utils

from pydbg import *
from pydbg.defines import *

snapshot_hook  = 0x00401450
restore_hook   = 0x004012B7
snapshot_taken = False
hit_count      = 0
address        = 0

########################################################################################################################
### callback handlers.
########################################################################################################################

def handle_bp (dbg):
    global snapshot_hook, restore_hook, snapshot_taken, hit_count, address

    if dbg.exception_address == snapshot_hook:
        hit_count += 1
        print "snapshot / mutate hook point hit #%d" % hit_count

        # if a process snapshot has not yet been taken, take one now.
        if not snapshot_taken:
            start = time.time()
            print "taking process snapshot...",
            dbg.process_snapshot()
            end = time.time() - start
            print "done. completed in %.03f seconds" % end

            snapshot_taken = True

        if hit_count > 1:
            if address:
                print "freeing last chunk at %08x" % address
                dbg.virtual_free(address, 1000, MEM_DECOMMIT)

            print "allocating chunk of memory to hold mutation"
            address = dbg.virtual_alloc(None, 1000, MEM_COMMIT, PAGE_READWRITE)
            print "memory allocated at %08x" % address

            print "generating mutant...",
            fuzz = "A" * 750
            random_index = random.randint(0, 750)
            mutant  = fuzz[0:random_index]
            mutant += chr(random.randint(32,126))
            mutant += fuzz[random_index:]
            mutant += "\x00"
            print "done. index at %d." % random_index

            print "writing mutant into target memory space"
            dbg.write(address, mutant)

            print "modifying function argument to point to mutant"
            dbg.write(dbg.context.Esp + 4, dbg.flip_endian(address))

            print "continuing execution...\n"
            dbg.bp_set(restore_hook)

    if dbg.exception_address == restore_hook:
        start = time.time()
        print "restoring process snapshot...",
        dbg.process_restore()
        end = time.time() - start
        print "done. completed in %.03f seconds" % end
        dbg.bp_set(restore_hook)

    return DBG_CONTINUE


def handle_av (dbg):
    print "***** ACCESS VIOLATION *****"

    crash_bin = utils.crash_binning.crash_binning()
    crash_bin.record_crash(dbg)

    print crash_bin.crash_synopsis()
    dbg.terminate_process()


########################################################################################################################


dbg = pydbg()

dbg.set_callback(EXCEPTION_BREAKPOINT,       handle_bp)
dbg.set_callback(EXCEPTION_ACCESS_VIOLATION, handle_av)

found_target = False
for (pid, proc_name) in dbg.enumerate_processes():
    if proc_name.lower() == "fuzz_server.exe":
        found_target = True
        break

if found_target:
    dbg.attach(pid)
    dbg.bp_set(snapshot_hook)
    dbg.bp_set(restore_hook)
    print "attached to %d. debugger active." % pid
    for addr in dbg.breakpoints.keys():
        print "bp at %08x" % addr
    dbg.run()
else:
    print "target not found."
