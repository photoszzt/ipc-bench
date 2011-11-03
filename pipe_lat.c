/* 
    Copyright (c) 2011 Anil Madhavapeddy <anil@recoil.org>

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use,
    copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following
    conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.
*/

#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/time.h>
#include <err.h>
#include <inttypes.h>
#include <netdb.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#include "test.h"
#include "xutil.h"

typedef struct {
  int ifds[2];
  int ofds[2];
} pipe_state;

static void
init_test(test_data *td)
{
  pipe_state *ps = xmalloc(sizeof(pipe_state));
  if (pipe(ps->ifds) == -1)
    err(1, "pipe");
  if (pipe(ps->ofds) == -1)
    err(1, "pipe");
  td->data = (void *)ps;
}

static void
run_child(test_data *td)
{
  pipe_state *ps = (pipe_state *)td->data;
  void *buf = xmalloc(td->size);
  int i;

  for (i = 0; i < td->count; i++) {
    xread(ps->ifds[0], buf, td->size);
    xwrite(ps->ofds[1], buf, td->size); 
  }
}

static void
run_parent(test_data *td)
{
  pipe_state *ps = (pipe_state *)td->data;
  void *buf = xmalloc(td->size);

  latency_test("pipe_lat",
    do {
      xwrite(ps->ifds[1], buf, td->size); 
      xread(ps->ofds[0], buf, td->size);
    } while (0),
    td
  );
}

int
main(int argc, char *argv[])
{
  test_t t = { init_test, run_parent, run_child };
  run_test(argc, argv, &t);
  return 0;
}
