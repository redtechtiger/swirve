# Swirve-Userclient
 
An open-source client and server to manage your Minecraft servers.
Swirve runs natively on Windows machines through WPF using a client
called `Userclient` and communicates with the remote server, called
`Framework`, through a custom TCP-based API. Userclient is written
in C# and the UI is styled through XAML, and Framework is written in
pure C++ and utilizes kernel calls, the proc filesystem, etc in
order to run and track the current Minecraft servers (`Nodes`).

Licenced under the MIT license. The codebase is full of bad practices,
unsafe code, race conditions and pure gibberish. Forks are more than
welcome, and so are pull requests. The final release in projected in
a couple of weeks, which will be Swirve v1.2.
