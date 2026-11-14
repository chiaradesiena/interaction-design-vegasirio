\---

name: pubblica

description: Manda su GitHub le modifiche dell'utente creando un branch, facendo commit e push e aprendo o aggiornando la pull request. Da usare quando l'utente accetta di aggiornare GitHub o scrive /pubblica.

\---

\# Pubblicare le modifiche su GitHub



Spiega ogni passo all'utente in una frase semplice, senza gergo.



1\. Se sei su `main`, crea un branch con `git switch -c <tipo>/<descrizione>`, dove tipo è `aggiunta`, `modifica`, `correzione`, `taratura` o `doc`, e la descrizione è breve, minuscola, senza accenti, con trattini (es. `aggiunta/stato-notte`, `taratura/molla-rotazione`).

2\. Mostra l'elenco dei file modificati. Se compaiono file inattesi (file temporanei, file molto pesanti, password o chiavi, modifiche a `EsploratoreCoreS3.ino` o a `.claude/settings.json`), segnalali e chiedi prima di includerli.

3\. Fai il commit con un messaggio chiaro in italiano.

4\. `git push -u origin HEAD`

5\. Se per il branch non esiste ancora una PR, creala con `gh pr create --base main`, con titolo e descrizione in italiano: cosa è cambiato, perché, quali costanti sono state toccate e cosa controllare sulla board.

&#x20;  Se esiste già, il push l'ha aggiornata.

6\. Dai all'utente il link della PR e spiega che Chiara (la proprietaria della repository) la controllerà prima che entri in `main`.



Se compaiono conflitti o errori, segui la sezione "Conflitti ed errori" del CLAUDE.md.

