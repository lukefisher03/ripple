# Terminal RSS Aggregator
A lightweight RSS Aggregator that runs in your terminal. Written in C

## Features
TBD

## TODO
### Not Started
- Add mechanism for storing RSS feeds (Database? Local files? Haven't figure out the scope yet. Database seems excessively complicated).
- Periodically fetch stored RSS feed links.
- UI.
    - Is the UI just going to be text? 
    - Will it be a legit TUI?
    - GUI? (doubt this will happen, but recording for completeness)
- Support windows?

### In Progress
- Parse RSS XML. (80% complete)
    - Handle CDATA properly. Should I figure out some way to display the HTML or just convert to plaintext?
- Build API for fetching RSS feeds using C network sockets.

### Complete
- [x] Break down RSS into tag objects.
- [x] Get HTTPS working with C network sockets.

## Notes
### RSS XML
#### Should I Build a DOM or AST?
No. The RSS protocol doesn't have arbitrary depth. The structure is known beforehand. I can simply iterate through the XML and look for `<item>` tags. This works well with a stack. Every time we see an opening `<item>` tag, we'll push to the stack. Then we'll continue parsing tags until we come across the closing `</item>` tag. If we see the closing tag, we know that item is complete and we can store the item struct.

### How Do You Handle CDATA Blocks?
The RSS standard says that you can include HTML in encoded `<description>` elements. This is obviously hard to render in the terminal. Some strategies on how to handle this could be partial rendering, converting to markdown, or removing the HTML all together. The selected strategy is yet to be determined.

### Will You Display Images?
Probably not. The only way I could see this happening is if I found a cool ASCII image converter or something like that. Open to ideas.

### What About XML Namespaces in RSS?
Namespaces are read by the parser. But I'm not sure how useful they'll be. I may use them to help with content tags. We'll see.