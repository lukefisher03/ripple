# Ripple RSS Aggregator

A lightweight RSS Aggregator that runs in your terminal. Written in C. Backed by a SQLite database to persistently store RSS channels and articles.

_Docker instructions at the bottom of the page_

See [todo.md](todo.md) for what's happened and what's next.
See [PAGES.md](PAGES.md) to take a brief tour of the application.

![Feed Reader](assets/feeds_page.png)

### Building Ripple

You'll need to clone the repository and include the `termbox2` submodule. You can clone and include submodules by default by using:

```
git clone --recurse-submodules https://github.com/lukefisher03/ripple
```

#### Using Docker Compose 
A dev image is provided to create an isolated environment to run tests and get up and running quickly. Run the commands below to get started.

```bash
docker compose up -d
```

Exec into the container. Modify `config/channel_list.txt` in the host to add new feeds. You can also see the debug log in the `config` directory.

```bash
docker compose exec ripple ./main config/channel_list.txt
```

Execute the test suite by running
```
docker compose run --rm test
```

Tear everything down, this will delete the database and any imported feeds will be lost.

```bash
docker compose down
```

#### Building from source

This project only has a few dependencies.

- sqlite3
- uriparser
- openssl
- gtest

Ensure that `pkg-config` can locate these dependencies and run
```
make main
./main <optional text file for importing new channels>
```

Alternatively, if debug symbols are required
```
make main_debug
./main_debug <optional text file for importing new channels>
```
