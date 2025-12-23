# DMD DiaRY

DRY – Diary and Research-log Keeping Utility

DRY aims to make it simple to keep and annotate daily videos/notes as a personal diary and research log. All files are stored in an encrypted directory. This is a work-in-progress (WIP) project, in a VERY EARLY STAGE OF DEVELOPEMENT, and for now, it only suits my basic needs.

Some operations may still need to be done manually, but the future goal is to rely completely on this tool (and on its Emacs integration, which will come after it reaches a minimally stable version) to track complete projects, research, and life, depending on the use case.

## FEATURES

- Create and delete diary entries (note and video)
- List and show entries
- Manage multiple diaries (project diary, daily, public)
- Full diary encryption
- WIP bash autocompletion

## USAGE

```shell
dry # get help command
dry init # initialize a diary (path) on current directory
dry add video [<path>] # register a video
dry add note [<path>] # add a text note

dry list date/+-timespan/today/yesterday # list entry in specified time span (WIP - only works with 'yesterday' or 'today')

dry show id [<path>] # show note by id (eg. dry show 2025-04-11.org [diary] )
dry play id [<path>] # play video by id - NOT IMPLEMENTED
dry delete id/date/span [<path>] # delete entry by id
```

## DEPENDENCIES

- libconfig
- ffmpeg
- encfs
- less
- exa
- ranger

## INSTALLATION

To install DRY clone the repo and run the following inside the root directory

```shell
cd dmd-diary
sudo make install
```

Than take a look at the config file and set your preferred values

## UNINSTALL

To remove dry run the following inside the root directory

```shell
sudo make uninstall
```

## CONFIG

Possible config paths (can have multiple config files)
- `~/.config/dry.yml`
- `~/dry.yml`
- `/etc/dry.yml`

```
default_diary = "diary";
text_editor = "emacsclient -t";
video_player = "mpv";
default_dir = "/home/$USER/.dry/storage"; # this path must to be absolute
```

## INTERNALS

DRY is a simple wrapper around a series of CLI operations, written in C because it's lighweight and I love it.

## TODOS

- [x] fix: write reference after successfully having saved the video
- [x] fix: exit if encfs fails to open properly
- [x] fix: folder managing
- [x] fix: ffmpeg not properly initialize webcam
- refactor: general cleanup
- [x] feat: Open/close diary to manual modify
- [x] fix: installation (config are exported wrong, diary init fails)
- fix: help arguments
- fix: list operations (today tomorrow etc)
- fix: fix bash completion
- feat: implement play
- fix: change defaults for better portability (eg. exa -> ls), also add options to configure it from the configs
- feat: add diary attachments (dry new attach path)
- feat: export diary to unencrypted form
- improve: CLI, use params to permit permutations (-m "message", -d YYYY-MM-DD) (argp.h)
- feat: more config for user to customize
- improve: main function and argment parsing modularity
- improve: chiper enc/dec optimization (modular fs? exclude big files? encfs support?)
- feat: make encryption optional
- feat: add a "use" command to temporary select a diary for that terminal session (maybe an env var?)
- improve:  ensure all commands supports the same interface (command subcommand [diary] [args]). 
            Looks like list work only with the default diary

Next steps:
- feat: add code logging feature
- feat: configure markdown (and others) as a possible choice for markup langs
- feat: filters
- improve: new note, video/attachments reference inserted into notes of day
- feat: mobile application integration
- feat: permits to embed screencast and screenshots, as well as other formats, to the video entry
- feat: permits to customize an overlay for the video entry in config file
- sec: security code review, focalized on enc and dec of data, as race condition that could let the db open
- feat: add colors
- feat: sftp support
- feat: run as a remote service (???)

Add configurations:
- video
  - size
  - format
  - webcam
  - microphone
- markdown formats (see `configure markdown`)
- display note command (`cat`, `less`, etc...)

## NOTES

For initilizing a new diary use: `init`

```shell
dry init <diary_name>
```

To create new content, issue the `add` command. DRY will create a reference in the current daily entry, and will create the entry itself if it doesn't already exist, marking the current time of day.
- video – Records a video using the webcam and microphone connected to the PC, and adds a reference to the video in the daily entry.
- note – Adds a text note to the daily entry.

```shell
dry new <video|note>
```

To explore captured content, use the `list`, `show` or `explore` commands:

- `list` : Lists the content related to the entries specified by the query.
- `show` : Displays a daily entry or plays back the attached content.
- `explore` : Open file manager (currently ranger) inside the unencrypted diary path.

Deleting a diary entry is done using the `delete` command.

```shell
dry delete <id> <diary_name>
```
