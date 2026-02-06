# Terminal RSS Aggregator
A lightweight RSS Aggregator that runs in your terminal. Written in C. Backed by a SQLite database to persistently store RSS channels and articles. 

*Dockerfile instructions at the bottom of the page*

### Main menu
![Main Menu](assets/main_menu.png)

### Feed reader
![Feed Reader](assets/feed_reader.png)

### Articles 
![Article Page](assets/article.png)

### Channels
Browse articles by channel, delete channels, or import new channels from this page.
![Channels Page](assets/view_channels.png)

### Channel
View the articles from a specific channel.
![Channel Page](assets/view_channel.png)

### Import channels
Load new channels via their links from this page. To import channels create a file and store the links to each channel separated by newlines. Start the application and provide the path to the file as the first argument. You will will see the links populate in this list. Press ENTER to import the new channels.
![Import Page](assets/import_channels.png)

See todo.txt for what's next

### Building Ripple
#### Using the Dockerfile
The `Dockerfile` uses a multistage Alpine Linux build. The first stage compiles the binary and the second produces the final image with just the binary and dependencies.

In this example, I'm using Podman, but the docker commands are very similar.

Build the image.
``` bash
podman build -t ripple .
```

Start a container.
``` bash
podman run -it --name rip ripple
```

From here you'll see the application start up. If you exit the application, you'll need to re-enter the container instead of creating a new one. 
``` bash
podman container start -ai rip
```

If you want to import new channels you need to exec into the container and modify the `channel_list.txt` file. While the container is running, run:
``` bash
podman container exec -it rip sh
vim channel_list.txt
```
After modifying the list, you will see updates in the channel import page. No need to exit the container and start it again, just go back a page and re-enter the import page.

#### Building from source
This project only has a few dependencies.
- sqlite3
- uriparser
- openssl

Ensure that `pkg-config` can locate these dependencies and run 
``` bash
make main
./main <optional text file for importing new channels> 
```