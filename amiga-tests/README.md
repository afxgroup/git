# Amiga clib4 Spawn/Pipe Tests

These tests are designed to validate whether `spawnvpe()` under clib4 preserves
stdio/pipe semantics expected by Git remote helpers.

## Files

- `spawnvpe_fd_inherit_test.c`: parent/child single-binary test.

## Build (Amiga)

Use your normal clib4 toolchain, for example:

```sh
gcc -Wall -Wextra -O2 spawnvpe_fd_inherit_test.c -o spawnvpe_fd_inherit_test
```

## Run

```sh
./spawnvpe_fd_inherit_test
```

## Expected success behavior

- Child reports stdin/stdout as non-tty when attached to pipes.
- Child reads one line from stdin and writes one response line to stdout.
- Parent receives response and waitpid() returns promptly.

## Failure patterns indicating clib4 regression

- Child reports stdin/stdout still tty despite pipe mapping.
- Parent write succeeds but child read blocks or times out.
- Child never emits stdout response and parent blocks on read/waitpid.
- Environment values are visible in parent but missing in child.
