from time import sleep
from npy import nstreamcom, terminal
import serial

def nstreamcom_test():
    port = serial.Serial('COM4', 1000000)
    last_toggle = 0
    board_time = 0
    led_state = True

    while True:
        recv = port.read_all()
        if recv is not None:
            data = nstreamcom.parse(bytearray(recv))
            if data is not None:
                board_time = int.from_bytes(data[1], 'little')
                terminal.clear()
                print(board_time)
                if (board_time - last_toggle >= 500):
                    last_toggle = board_time
                    led_state = not led_state
                    port.write(nstreamcom.parse(( 1, led_state.to_bytes(1, 'little'), 1 )))

def terminal_app_test():
    app = terminal.TerminalApp("This App", ' => ')

    def fun(args):
        print('Fun!')

    def args_test(args):
        print(args.args)

    def loading_bar_test(args):
        print(f'Setting up { args.args }...')
        for i in range(101):
            terminal.loading_bar(i, clear_on_each=True)
            sleep(0.5)
        print(f'Done { args.args }!')

    app.commands.append(terminal.TerminalCommand('q', exit, 'Exit'))
    app.commands.append(terminal.TerminalCommand('exit', exit, 'Exit'))
    app.commands.append(terminal.TerminalCommand('clear', terminal.clear, 'Clear terminal output'))

    app.commands.append(terminal.TerminalCommand('fun', fun, 'Test cb function'))
    app.commands.append(terminal.TerminalCommand('args-test', args_test, 'Test the arguent and flags handling'))
    app.commands.append(terminal.TerminalCommand('loading-bar-test', loading_bar_test, 'Test the loading bar'))

    app.start()

nstreamcom_test()