# Shelltester

Dieses Tool kann verwendet werden, um die Funktionen der Testat-Shell
zu testen. Es führt die Shell aus und wendet eine Reihe von Eingaben
an, deren Ergebnis ausgewertet und auf Korrektheit wird.

Das Tool besteht aus einem kleinen Shellscript, das wiederum ein TCL-Skript
(harness.tcl) nutzt um Ein- und Ausgaben mit Hilfe des Programms `expect` 
durchzuführen und auf Korrektheit zu prüfen. Um Timeouts zu implementieren
wurde eine Hilfsfunktion in C implementiert. Bei der ersten Ausführung wird
der C-Programmcode automatisch übersetzt.

## Installation

Das Programm `expect` und ein `tcl` Interpreter müssen installiert sein.

## Benutzung

```
./shtest </pfad/zur/shell>
```

Das Programm führt folgende Tests durch:

| Test                                | Beschreibung                               |
| ----------------------------------- | ------------------------------------------ |
| 00-basic-fork-exec.t                | Prüft ob sich grundsätzlich ein existierendes Programm mit absoluter Pfadangabe und ohne Optionen ausführen lässt. |
| 01-commands-with-parameters.t       | Prüft ob sich Programme mit Parametern ausführen lassen. |
| 02-commands-invoked-from-path.t     | Prüft ob sich Programme auch ohne absolute Pfadangabe ausführen lassen. |
| 03-cd-changes-directory.t           | Prüft ob sich mit Hilfe von `cd` das Verzeichnis wechseln lässt. | 
| 04-ret-displays-exitstatus.t        | Prüft ob der exit-status des letzten Kommandos mit `ret`anzeigen lässt. |
| 05-semicolon-separates-commands.t   | Prüft ob mehrere Kommandos mit Semikolon getrennt angegeben werden können. |
| 06-piped-commands.t                 | Prüft ob sich genau zwei Programme mit einer Pipe verbinden lassen. |
| 07-exit-shell.t                     | Prüft ob sich die Shell korrekt beendet. |

## Quelle

Dies ist eine stark vereinfachte Variante einer Toolchain zum Testen von
selbst getüftelten Shells. Wer an der Shell weiter arbeiten möchte, sollte
sich auf jeden Fall dieses Github Repository ansehen:

https://github.com/tokenrove/build-your-own-shell
