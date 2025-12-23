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

dry show id|today|yesterday [<path>] # show note by id (eg. dry show 2025-04-11.org [diary] )
dry delete id/date/span [<path>] # delete entry by id
```

## DEPENDENCIES

**Required:**
- libconfig
- encfs
- xdg-utils (for xdg-open)

**Optional (for video recording):**
- ffmpeg

**Optional (configurable alternatives):**
- pager: less, more, cat (default: less)
- file manager: ranger, nautilus, dolphin (default: xdg-open)
- list command: exa, lsd, ls (default: ls -lah)

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
default_diary = "diary"
default_dir = "/home/$USER/.dry/storage"  # this path must be absolute

# Optional settings (defaults shown)
text_editor = "vi"
video_player = "xdg-open"
list_command = "ls -lah"
file_manager = "xdg-open"
pager = "less"
```

DRY will search for config files in the order shown above, and will merge them, with the latter having precedence over the former.

DRY has a terminal bash completion script located in the `completion` file. To enable it, source it in your `.bashrc` or equivalent shell configuration file:

```shell
source /path/to/dmd-diary/completion
```

A simple terminal integration can be configured by adding the following line to your shell configuration file:

```shell
# Add to ~/.bashrc or ~/.zshrc
PS1='$(dry status)$ '
# or for zsh right prompt
RPROMPT='$(dry status)'
```

## INTERNALS

DRY is a simple wrapper around a series of CLI operations, written in C because it's lighweight and I love it.

## TODOS

- [x] fix: write reference after successfully having saved the video
- [x] fix: exit if encfs fails to open properly
- [x] fix: folder managing
- [x] fix: ffmpeg not properly initialize webcam
- [x] refactor: general cleanup
- [x] feat: Open/close diary to manual modify
- [x] fix: installation (config are exported wrong, diary init fails)
- [x] fix: help is not shown when --help or -h
- [x] improve: CLI, use params to permit permutations (-m "message", -d YYYY-MM-DD) (argp.h)
- [x] fix: list operations (today tomorrow etc)
- [x] fix: fix bash completion
- [x] fix: change defaults for better portability (eg. exa -> ls), also add options to configure it from the configs
- [x] improve: main function and argment parsing modularity
- [x] improve:  ensure all commands supports the same interface (command subcommand `[diary]` `[args]`). Looks like list work only with the default diary
- [x] feat: add tests
- [x] feat: implement play command (`dry play <id>|today|yesterday`). Play video entries. If the user passes today/yesterday it plays all videos recorded that day, in order of recording, otherwise it plays the video with the specified id.
- [ ] feat: unlock command to open the diary for manual modification (`dry unlock [-d diary]`), which will decrypt the diary,and a lock command to close it again (`dry lock [-d diary]`).
- [ ] fix: delete command
- [ ] feat: implement a command to delete the whole diary (`dry delete -d diary_name --all`), with confirmation prompt
- [ ] feat: add a "use" command to temporary change the default diary for that terminal session (maybe an env var?)
- [ ] feat: add diary attachments (dry new attach path)
- [ ] feat: make encryption optional (configurable per diary?)
- [ ] feat: make command to export diary to unencrypted form, the command supports exporting the whole diary or a time span (dry export [--from YYYY-MM-DD] [--to YYYY-MM-DD] [-d diary])
- [ ] feat: import command to import unencrypted diaries into encrypted form (dry import path [-d diary])
- [ ] feat: more config for user to customize
- [ ] improve: chiper enc/dec optimization (modular fs? exclude big files? encfs support?)

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
