# CLAUDE.md — Tamagotchi Stella (M5Stack CoreS3)

Progetto di gruppo per il corso di Interaction Design. Utente: **Chiara**, programmatrice alle prime armi.
Tutto il codice lo scrive Claude; Chiara lo carica sulla board e lo tara.

Progetto di gruppo per il corso di Interaction Design. Utenti: **Chiara** (proprietaria della repository, GitHub: `CHIARA_USERNAME`) e le compagne del gruppo, tutte programmatrici alle prime armi.
Nel resto delle istruzioni "Chiara" indica chi sta usando Claude in quel momento, salvo nella sezione "Collaborazione con git" e nella skill `pubblica`, dove indica solo la proprietaria.
Tutto il codice lo scrive Claude; Chiara lo carica sulla board e lo tara.

Altre istruzioni:
- `.claude/rules/progetto.md`: obiettivo, file, automa a stati, estetica, domande aperte.
- `.claude/rules/codice.md`: hardware, architettura, stile di programmazione (caricato quando apri un `.ino` o un `.h`).
- skill `pubblica`: procedura per mandare le modifiche su GitHub.

## Flusso di lavoro (OBBLIGATORIO)
Quando Chiara chiede di programmare qualcosa:
1. **Non scrivere codice subito.** Presenta un **piano a fasi** (breve, numerato): cosa si fa in ogni fase, quali moduli/file si toccano o si creano, cosa Chiara vedrà sul dispositivo a fine fase, eventuali domande aperte.
2. **Aspetta l'approvazione esplicita.** Implementa solo le fasi approvate, una alla volta se così richiesto.
3. Dopo l'implementazione dai la **spiegazione semplice** e l'**elenco costanti** (formato sotto).
4. Se mancano informazioni, fai domande prima del piano. Non fare ipotesi su design, soglie o comportamento della creatura.

Eccezione: correzioni minime di bug già concordate non richiedono un piano completo, basta dire cosa si cambia.

## Aiutare chi scrive la richiesta
Quasi nessuno nel gruppo sa programmare o scrivere prompt. Una richiesta di codice è completa quando dice 4 cose:
1. **Quale stella**: Sirio, Vega o tutte e due.
2. **Cosa succede**: cosa fa la stella (es. "diventa più piccola", "lampeggia", "cambia colore").
3. **Quando**: cosa lo fa partire (un tocco, una voce, una scossa, il buio, il tempo che passa, l'altra stella vicina).
4. **Nuovo o da cambiare**: è una cosa nuova, oppure si modifica una cosa che c'è già (e cosa non va oggi).

Esempio da mostrare: *"Su Vega: quando qualcuno parla vicino, la stella pulsa più veloce per qualche secondo. È una cosa nuova."*

- Se manca una voce, non indovinare: chiedi solo quella, con 2–3 esempi tra cui scegliere.
- Se la richiesta è molto vaga, mostra l'elenco e l'esempio qui sopra come modello da copiare.
- Se qualcosa non funziona, chiedi: cosa ti aspettavi, cosa succede invece, cosa hai fatto prima.

## Formato della risposta dopo ogni implementazione
- **Spiegazione a grandi linee**, in italiano semplice, senza gergo: cosa fa il codice, in 3–8 righe. Se serve un termine tecnico, spiegarlo in una frase (come fanno i commenti di `Sirio/Sirio.ino`).
- **Costanti da cambiare**: tabella con nome, valore attuale, cosa succede se si alza/abbassa, modulo/file dove si trova. Sono solo quelle che Chiara ha senso tarare.
- **Cosa controllare sulla board** (cosa dovrebbe vedere; cosa fare se un movimento va al contrario, es. cambiare `SEGNO_ASSE_*`).
- Evidenziare i file modificati e quelli nuovi.

## Regole di modifica
- **Cambia meno codice pregresso possibile.** Aggiungi moduli/funzioni nuovi invece di riscrivere. Non rinominare, non riformattare, non "ripulire" codice che funziona. Modifiche a codice esistente solo dove necessario, e dichiaralo.
- Aggiungere una funzionalità = nuovo file/modulo + poche righe di collegamento (un `case`, una chiamata) nei punti di aggancio.
- Non aggiungere dipendenze o librerie nuove senza chiederlo.
- Mai tarare "a occhio" in silenzio: ogni numero tarabile va in una costante in configurazione.

## Collaborazione con git
La repository è condivisa dal gruppo. Le compagne di Chiara non devono imparare git: lo gestisci tu per loro, spiegando ogni passo in una frase semplice e senza gergo. La prima volta che nomini una pull request, chiamala "proposta di modifica (pull request)".

### Setup (all'inizio di ogni sessione, prima di tutto il resto)
1. Esegui `gh --version`. Se `gh` non è installato, spiega come installarlo (Windows: `winget install --id GitHub.cli`; macOS: `brew install gh`) e fermati finché l'utente non l'ha fatto e riaperto il terminale.
2. Esegui `gh auth status`. Se l'utente non è autenticato:
   - spiega in una frase che serve collegare il PC al suo account GitHub;
   - esegui `gh auth login --web --hostname github.com --git-protocol https`;
   - mostra all'utente il codice di una sola volta che compare nell'output e digli di inserirlo nella pagina del browser, poi attendi che confermi;
   - se il comando resta bloccato o fallisce, chiedi all'utente di eseguirlo lui stesso in un terminale separato e di tornare quando ha finito.
3. Esegui `gh auth setup-git`, così git usa le stesse credenziali di `gh`.
4. Controlla `git config user.name` e `git config user.email`; se mancano, chiedi nome ed email all'utente e impostali con `git config --global`.
5. Esegui `gh api user --jq .login` per sapere chi sta lavorando. Se è `CHIARA_USERNAME`, il resto di questa sezione vale solo se lei lo chiede.

### Allineamento con GitHub (subito dopo il setup)
1. Esegui `git fetch --prune` e `git status`.
2. Se sei su un branch diverso da `main`, controlla la PR con `gh pr view --json state,url`:
   - se è `MERGED` o `CLOSED` e non ci sono modifiche non salvate: `git switch main`, `git pull`, `git branch -D <branch>`, e avvisa l'utente in una frase;
   - se è ancora aperta: ricorda che c'è una proposta in attesa e chiedi se vuole continuare su quella o iniziare un lavoro nuovo (in quel caso torna su `main` e aggiornalo).
3. Se sei su `main`, esegui `git pull`.

### Durante il lavoro
- Non fare mai commit né push su `main`.
- Quando un insieme di modifiche è completo (di solito dopo la risposta di fine implementazione e dopo che l'utente ha provato sulla board), chiedi: "Vuoi mandare queste modifiche su GitHub?" Se l'utente accetta, usa la skill `pubblica`.

### Conflitti ed errori
- Se una PR aperta va in conflitto con `main`, aggiorna il branch con `git merge origin/main`.
- Non risolvere conflitti in silenzio: spiega quali file sono coinvolti, proponi una soluzione e chiedi conferma. In `config.h` i conflitti riguardano spesso tarature diverse della stessa costante: chiedi quale valore tenere invece di sceglierlo tu.
- I PDF e le immagini non si possono unire: in caso di conflitto chiedi quale versione tenere.
- Mai eseguire `git push --force`, `git reset --hard`, `git clean`, `git rebase` o cancellare branch remoti senza una richiesta esplicita dell'utente.
- Se qualcosa non torna, fermati, spiega cosa è successo e suggerisci di sentire Chiara.