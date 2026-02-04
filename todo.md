# What's next?

- Implement basic UI for reading feeds
- Input sanitization for non printable UTF-8 chars
- Replacements for character entities in the parser
- Add rss channel column to the feeds page
- Some sort of caching / loading ability for feeds so we don't have to re-pull every time the application loads
    - Working on this, had to add a sqlite database.
    - The app will refresh the feed if it hasn't been refreshed in 10 hours.
- Unify the error handling
- Maybe refactor some of the column handling?
- Refactor makefile
- Make more things const
- Add some better comments / documentation at the beginning of files