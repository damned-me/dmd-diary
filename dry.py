#! /bin/python3
import argparse
from os import replace
import pathlib
import configparser
import subprocess
import datetime
import sys
import signal

commands = ['init',
            'new',
            'list',
            'show',
            'explore',
            'delete']
usage='''
    new    New entry for specified diary
    delete Delete specified entry
    show   Show entries by day
    list   List entries by datespan
    init   Init new diary'''

class Dry(object):
    def __init__(self):
        # CONFIGS
        self.config = configparser.ConfigParser()
        confpath = pathlib.Path('dry.conf')

        self.config.read(confpath)


        # ARGUMENT PARSING
        parser = argparse.ArgumentParser(
            prog='dry',
            description='DiaRY is a CLI journaling utility',
            epilog='author: damned-me')

        parser.add_argument('command',
                            type=str,
                            help='Subcommand to run',
                            choices=['new', 'show', 'explore', 'list', 'init'])
        parser.add_argument('--version',
                            help='show %(prog)s version',
                            action='version',
                            version='%(prog)s 1.0')
        parser.add_argument('-v', '--verbose',
                            help='show verbose log',
                            action='store_true')

        args = parser.parse_args(sys.argv[1:2])

        # VARIABLES
        self.storage = self.config['Defaults']['Storage']
        self.name = self.config['Defaults']['Diary']

        self.encpath = pathlib.Path(self.storage) / f'.{self.name}'
        self.path = pathlib.Path(self.storage) / self.name


        #if not args.command in commands:
        #    print('Unrecognized command')
        #    parser.print_help()
        #    exit(1)
        if not args.command == 'init':
            self.open()
            getattr(self, args.command)()
            self.close()
        else:
            self.init()

    def open(self):
        if subprocess.call(['encfs', self.encpath, self.path]):
            exit(1)

    def close(self):
        subprocess.call(['fusermount', '-u', self.path])

    def init(self):
        parser = argparse.ArgumentParser(
            description='Initialize new diary')

        parser.add_argument('name',
                            type=str,
                            help='Name of new diary')

        args = parser.parse_args(sys.argv[2:])
        enc = pathlib.Path(self.storage) / f'.{args.name}'

        if enc.is_dir():
            print('Diary already exists')
            return

        subprocess.call(['encfs',
                         '--paranoia',
                         enc,
                         pathlib.Path(self.storage) / args.name])

    def new(self):
        parser = argparse.ArgumentParser(
            description='Create new entry')

        parser.add_argument('type',
                            type=str,
                            help='video or note',
                            choices=['video', 'note'])

        args = parser.parse_args(sys.argv[2:])

        ts = datetime.datetime.now()

        fpath = self.path / ts.strftime('%Y/%m/%d')
        fpath.mkdir(parents=True, exist_ok=True)

        fname = ts.strftime('%Y-%m-%d') + '.org'
        fpath = fpath / fname

        if not fpath.is_file():
            with open(fpath, 'w') as f:
                f.write(f"* {ts.strftime('%Y-%m-%d')}\n")

        if args.type == 'video':
            self.video()
        elif args.type == 'note':
            self.note()

    def note(self):
        ts = datetime.datetime.now()

        # file name : yyyy-mm-dd.ext
        fname = ts.strftime('%Y-%m-%d') + '.org'

        # file path : storage/name/yyyy/mm/dd/fname (yyyy-mm-dd.ext)
        fpath = self.path / ts.strftime('%Y/%m/%d') / fname

        with open(fpath, 'a') as f:
            f.write(f"** {ts.strftime('%H:%M:%S')}\n")

        subprocess.call(['emacsclient', '-t', fpath])

    def video(self):
        ts = datetime.datetime.now()

        fname = ts.strftime('%Y-%m-%d') + '.org'
        fpath = self.path / ts.strftime('%Y/%m/%d') / fname

        vname = ts.strftime('%Y-%m-%d_%H-%M') + '.mkv'
        vpath = self.path / ts.strftime('%Y/%m/%d') / vname

        # Check if file already exists before writing reference and start recording
        if vpath.is_file():
            print('Error, file already exist, this is actually really weird, is your system time correctly set?')
            return

        # add reference to daily entry
        with open(fpath, 'a') as f:
            f.write(f"** {ts.strftime('%H:%M:%S')}\n")
            f.write(f"file:{vname}\n")
        try:
            subprocess.call(['ffmpeg',
                             '-f', 'v4l2',
                             '-framerate', '30',
                             '-video_size', '1024x768',
                             '-input_format', 'mjpeg',
                             '-i', '/dev/video0',
                             '-f', 'pulse',
                             '-ac', '1',
                             '-i', 'default',
                             '-c:a', 'pcm_s16le',
                             '-c:v', 'mjpeg',
                             '-b:v', '64000k',
                             vpath,
                             '-map', '0:v',
                             '-vf', 'format=yuv420p',
                             '-f', 'xv', 'display'])
        except KeyboardInterrupt:
            print(f'Video saved in {vpath}')

    def explore(self):
        subprocess.call(['ranger', self.path])

    def list(self):
        ts = datetime.datetime.now()
        subprocess.call(['exa', self.path / ts.strftime('%Y/%m/%d')])

    def show(self):
        parser = argparse.ArgumentParser(
        description='Show entries for specified date')

        parser.add_argument('file',
                            type=str)

        args = parser.parse_args(sys.argv[2:])

        path =  self.path / self.name
        print(path)

        subprocess.call(['exa', '-l', path])

if(__name__ == "__main__"):
    Dry()
