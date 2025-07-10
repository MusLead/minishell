# Bericht zur Mini-Shell-Implementierung

**Name:** Agha Muhammad Aslam
**Studiengang:** B.Sc. Angewandte Informatik

## Update (Mac-Test) 10.07.2025

The minishell has been tested on mac. It works, but oother than the wc.
Somehow it does not work for the wc cases whether using pipe or not.
Because it takes a lot of time, it will not be continued to be developed or revised.

## Ziel der Aufgabe

Ziel war es, eine einfache Shell in C zu implementieren, die grundlegende Shell-Funktionalitäten wie das Ausführen von Programmen, das Wechseln von Verzeichnissen (cd), das Beenden (exit), das Anzeigen des letzten Rückgabewerts (ret), sowie die Nutzung von Pipes und das Ausführen mehrerer Kommandos pro Eingabe (mit `;` getrennt) unterstützt.

## Funktionsumfang

* **Kommandozeilen-Prompt** mit Anzeige des aktuellen Verzeichnisses.
* **Ausführung externer Programme** via `fork()` und `execv()`/`execvp()`.
* **Verzeichniswechsel** mit dem eingebauten Befehl `cd`.
* **Shell-Beenden** mit dem eingebauten Befehl `exit`.
* **Anzeige des letzten Rückgabewerts** mit `ret`.
* **Signalbehandlung für `Ctrl+C`**, um versehentliches Beenden zu vermeiden.
* **Unterstützung für einfache und mehrfach verschachtelte Pipes** (`|` zwischen mehreren Programmen).
* **Mehrere Befehle pro Eingabezeile** mit `;` getrennt.
* **Fehlerprüfung bei ungültigen Pipe-Eingaben** wie `| wc`, `ps aux | | wc` etc.



## Probleme und deren Lösungen

### 1. **`execv()` beendet die Shell nach Ausführung**

* **Problem:** Ohne `fork()` ersetzt `execv()` den laufenden Prozess.
* **Lösung:** Ein `fork()` erzeugt einen Kindprozess, in dem `execv()` aufgerufen wird. Der Elternprozess wartet mittels `waitpid()`.

### 2. **Fehlerhafte Argumentparsing bei `strtok()`**

* **Problem:** Nutzung von `strtok()` ohne Kopie des Strings kann Speicherzustand unerwartet verändern.
* **Lösung:** Kopie der Zeichenkette und Nutzung von `strtok_r()` zur Thread-sicheren Zerlegung.

### 3. **~Doppelte Prompt-Anzeige bei `Ctrl+D` (EOF)~**

* **WARNING, THIS CASE HAS BEEN DELETED**
* **Problem:** Nach `Ctrl+D` wurde zweimal der Prompt angezeigt.
* **Lösung:** Es wurde eine `has_children()`-Funktion implementiert, um sicherzustellen, dass das Prompt nur angezeigt wird, wenn keine Kindprozesse laufen.

### 4. **Ungültige Pipe-Syntax**

* **Problem:** Eingaben wie `| wc` oder `ps aux | | wc` wurden verarbeitet, obwohl sie ungültig sind.
* **Lösung:** Die `handle_multi_pipe()` Funktion prüft nun auf Leerkommandos vor, nach oder zwischen Pipes und gibt entsprechende Fehlermeldungen aus.

### 5. **Gedoppelte Prozesse und Kindzählung**

* **Problem:** Ohne Verwaltung von Kindprozessen kam es zu doppelten Prompts und unklarem Prozessstatus.
* **Lösung:** Eine globale `child_count` Variable trackt aktive Kindprozesse, was die Kontrolle verbessert.

## Technische Umsetzung

* Ein zentrales `while`-Loop verarbeitet Benutzereingaben.
* Kommandos werden mit `strtok_r` gesplittet.
* Pipes werden durch Trennung an `|` erkannt und in einer robusten, rekursiven Funktion `handle_multi_pipe()` verarbeitet.
* Fehlerhafte Pipes und Leerbefehle werden nun korrekt abgefangen.
* Alle benutzerdefinierten Kommandos (`cd`, `exit`, `ret`) werden vor der Programmsuche behandelt.
* Rückgabewerte von Programmen werden global in `last_status` gespeichert.
* Verbesserte Kommentierung und Formatierung nach C-Konvention.

## Erweiterung vom 10.06.2025

### Verbesserungen und neue Funktionen

1. **Multi-Pipe-Support erweitert**
   Die Shell unterstützt nun unbegrenzt viele verkettete Pipes (`ps aux | grep ssh | wc -l`) durch dynamische Pipe-Verwaltung mit `malloc()`.

2. **Fehlersichere Eingabevalidierung**
   Die Funktion `handle_multi_pipe()` erkennt und blockiert ungültige Pipe-Konstrukte wie:

   * `| wc`
   * `ls |`
   * `ps aux | | wc -l`
   * `| grep root | wc`
     Diese Eingaben werden mit klaren Fehlermeldungen quittiert.

3. **Dynamischer Speicher für Pipes**
   Statt fester Pipe-Arrays wird nun Speicher dynamisch mit `malloc()` reserviert, was die Skalierbarkeit und Speicherökonomie erhöht.

4. **Kommentierte Quelltexte**
   Jeder relevante Codeblock ist nun kommentiert, um Nachvollziehbarkeit und Wartbarkeit zu erhöhen. Kommentare folgen dem Muster `// >` und dokumentieren technische und semantische Entscheidungen.

## Offene Fragen

* Ist das gewählte Design des Prompts verständlich und benutzerfreundlich?
* Wären Erweiterungen wie Autovervollständigung oder History sinnvoll?

## Fazit

Die Mini-Shell erfüllt alle gestellten Anforderungen. Besonders hervorzuheben sind die Erweiterung auf mehrfache Pipes, die sichere Kindprozess-Verwaltung sowie die strukturierte Fehlerbehandlung bei Benutzereingaben. Die Shell ist modular, erweiterbar und stabil im Verhalten.

Für die Zukunft wären Features wie History, Autovervollständigung oder IO-Redirection denkbar.

---

Falls gewünscht, kann dieser Bericht noch um Diagramme (z.B. Prozessstruktur oder Flussdiagramm) ergänzt werden.
