import subprocess
import sys
import json
import paramiko
import os

result = ' '.join(sys.argv)

class Command:
    cmd: str = ''
    get_res: bool = True
    def __init__(self, a_cmd: str, a_get_res: bool = True) -> None:
        self.cmd = a_cmd
        self.get_res = a_get_res
        return
    
    def is_empty(self):
        return len(self.cmd) == 0

class Action:
    def __init__(self) -> None:
        return
    
    def getHosts(self):
        with open("./hosts.json") as f:
            json_root = json.load(f)
            f.close()

            if "hosts" in json_root:
                return json_root["hosts"]
        return None
    
    def execute_command_on_host(self, host: str, cmd: str, get_res: bool) -> list[str]:
        client = paramiko.SSHClient()
        client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        key_path = '/home/ubuntu/.ssh/cp_rsa'
        client.connect(hostname=host, username='ubuntu', key_filename=key_path, port=22)

        stdin, stdout, stderr = client.exec_command(cmd)

        if not get_res:
            client.close()
            return []
        # err_lines = stderr.readlines()
        std_lines = stdout.readlines()

        client.close()

        # if len(err_lines) > 0:
        #     return filter_returns_from_strings(err_lines)

        return filter_returns_from_strings(std_lines)

    def execute(self):
        cmds = self.commands()
        if len(cmds) == 0:
            return
        
        hosts = self.getHosts()
        if hosts is None:
            return

        responses = dict()

        
        for host in hosts:
            print('Host: ' + host)

            for cmd in cmds:
                assert isinstance(cmd, Command)
                if cmd.is_empty():
                    continue
                print('  Command: ' + cmd.cmd)
                lines = self.execute_command_on_host(host, cmd.cmd, cmd.get_res)
                for line in lines:
                    print('    ' + line)
        return


    def commands(self) -> list[Command]:
        return []
    
    @classmethod
    def create(cls, name: str):
        if name == "start":
            return StartAction()
        elif name == "stop":
            return StopAction()
        elif name == "status":
            return StatusAction()

        return Action()
    

def filter_returns_from_strings(raw_lines: list[str]) -> list[str]:
    lines = list()
    for raw_line in raw_lines:
        line = raw_line
        assert isinstance(line, str)
        while line.endswith('\n'):
            line = line[0:-1]

        lines.append(line)
    return lines

    
class StartAction(Action):
    def __init__(self) -> None:
        super().__init__()
        return
    
    def commands(self) -> list[Command]:
        return [
            Command('kill -9 `pidof manager`'),
            Command('kill -9 `pidof camerapro`'),
            Command('kill -9 `pidof datapro1`'),
            Command('/usr/local/bin/manager --start', False),
            Command('/usr/local/bin/manager --status')
        ]
    
class StopAction(Action):
    def __init__(self) -> None:
        super().__init__()
        return

    def commands(self) -> list[Command]:
        return [Command('/usr/local/bin/manager --stop')]

class StatusAction(Action):
    def __init__(self) -> None:
        super().__init__()
        return

    def commands(self) -> list[Command]:
        return [Command('/usr/local/bin/manager --status')]


def main():
    if len(sys.argv) < 2:
        print("No argument")
        return
    
    action = Action.create(sys.argv[1])
    action.execute()
    return

main()