# What's next?
Not in any particular order. Although, things prefixed with a double bang are important.

- [x] Everything up to this point.
- [x] Implement basic UI for reading feeds.
- [x] Add SQLite database plus API.
- [x] Channel importing and deleting.
- [x] View articles from a single channel.
- [x] Exclude certain articles from the feed.
- [ ] Input sanitization for non printable UTF-8 chars.
- [x] Replacements for character entities in the parser. (there are still more potential entity replacements, but I'm handling a significant portion of them).
- [ ] !!Unify the error handling!
- [x] Refactor makefile.
- [x] !!Build a docker container for fast spin up and testing.
- [ ] Add help page.
- [ ] Add tests.
- [ ] Add more support docs and make the documentation look better.
- [x] Hit it with valgrind for any mem leaks. (used leaks, the tools from Apple. Valgrind doesn't work on Apple silicon.)
- [ ] Refresh shouldn't delete the entire channel, it should check if it exists, and then update instead.