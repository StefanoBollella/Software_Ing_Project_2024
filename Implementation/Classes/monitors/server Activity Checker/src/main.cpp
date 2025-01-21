#include "main.h"
#include <optional>
#define MIN_ARG                                                                \
  8 // Almeno 8 argomenti: exec_file, pong_supplier, pong_customer,
    // pong_carrier, ping_supplier, ping_customer, ping_carrier, blocking_time
#define STREAM_MAX_LEN 50
#define MAX_BLOCKING 10000
#define STREAM_IND 0
#define MAX_NUM_VAL_MSG 2
#define SERVER_SUPPLIER "server_supplier"
#define SERVER_CUSTOMER "server_customer"
#define SERVER_CARRIER "server_carrier"
#define NUM_SERVERS 3

using namespace std;

namespace {

redisContext *c2r;
redisReply *reply;

char *pong[NUM_SERVERS]; // stream di risposta dei server al PING del monitor
char *ping[NUM_SERVERS]; // una strem di richiesta PONG per ciascun server

int block;
int pid;
long int iteration = 0;
char buf[200];
// variabili per il parsing dei messaggi
size_t num_msg;
size_t num_val_msg;

char server_id[100];
char value[100];

const char HOST_NAME[] = "localhost";
const unsigned int PORT_NUM = 6379;
const char CONSUMER_GROUP[] = "monitor-grp-0";

std::unordered_map<std::string, int> server_status;
Con2DB logdb("localhost", "5432", "logdb_usr", "47002", "logdb");

// Inizializza lo strem redis
inline void init_stream_redis() {

  c2r = redisConnect(HOST_NAME, PORT_NUM);

  if (c2r == nullptr || c2r->err) {
    if (c2r) {
      std::cerr << "initRedisStreams(): Error: " << c2r->errstr << std::endl;
    } else {
      std::cerr << "initRedisStreams(): Can't allocate redis context"
                << std::endl;
    }
    exit(EXIT_FAILURE);
  }
  cout << "main(): pid: " << pid << " connected to redis ..." << endl;

  // Verifica l'esistenza dello stream pong
  for (int i = 0; i < NUM_SERVERS; i++) {

    if (streamExists(c2r, pong[i])) {
      // Se esiste, lo elimina
      reply = (redisReply *)redisCommand(c2r, "DEL %s", pong[i]);
      printf("stream eliminata\n");
      assertReply(c2r, reply); // Verifica che ci sia una risposta valida
      // dumpReply(reply, 0);
      freeReplyObject(reply);
    }

    // Inizializza lo stream  pong con il groupName monitor-grp-0
    initStreams(c2r, pong[i], CONSUMER_GROUP);

    if (!streamExists(c2r, pong[i])) {
      std::cerr << "Error: Failed to initialize pong stream." << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  // Verifica l'esistenza degli stream ping
  for (int i = 0; i < NUM_SERVERS; i++) {

    if (streamExists(c2r, ping[i])) {
      // Se esiste, lo elimina
      reply = (redisReply *)redisCommand(c2r, "DEL %s", ping[i]);
      assertReply(c2r, reply); // Verifica che ci sia una risposta valida
                               // dumpReply(reply, 0);
      freeReplyObject(reply);
    }

    // Inizializza lo stream  ping con il groupName
    initStreams(c2r, ping[i], CONSUMER_GROUP);

    if (!streamExists(c2r, ping[i])) {
      std::cerr << "Error: Failed to initialize ping streams." << std::endl;
      exit(EXIT_FAILURE);
    }
  }
} // init

// Funzione per inizializzare lo stato dei server
inline std::unordered_map<std::string, int> initializeServerStatus() {
  return {{SERVER_SUPPLIER, 0}, {SERVER_CUSTOMER, 0}, {SERVER_CARRIER, 0}};
}

} // namespace

/**
 * @brief Verifica lo stato dei server e genera un messaggio di alert in base
 * alla risposta o al ritardo.
 *
 * La funzione controlla lo stato di ciascun server (server_supplier,
 * server_customer, e server_carrier) in base alla mappa `serverStatus`. Il
 * monitor assegna un valore di stato a ciascun server:
 * - `0` indica che il server non ha risposto,
 * - `1` indica che il server è attivo (ha risposto entro la finestra
 * temporale),
 * - `-1` indica che il server ha risposto con un ritardo di una finestra
 * temporale.
 *
 * In caso i server non rispondono o sono in ritardo, la funzione genera un
 * alert specifico per ciascun server. Se un server è attivo, viene segnalato
 * come tale.
 *
 * @param serverStatus Mappa che contiene i nomi dei server come chiave e i
 * rispettivi stati come valori.
 */
void alertMissingServers(
    const std::unordered_map<std::string, int> &serverStatus) {
  std::string output = "\n";

  bool isAWarningLog = false;

  // Aggiungi lo stato di ciascun server all'output
  for (const auto &server :
       {SERVER_SUPPLIER, SERVER_CUSTOMER, SERVER_CARRIER}) {
    output += std::string(server) + ": ";

    if (serverStatus.at(server) == 0) {
      output += "non-responsive";
      isAWarningLog = true;
    } else if (serverStatus.at(server) == 1) {
      output += "active";
    } else if (serverStatus.at(server) == -1) {
      output += "late";
      isAWarningLog = true;
    } else {
      output += "unknown";
      isAWarningLog = true;
    }
    output += " \n";
  }

  printf("%-15s\n", output.c_str());

  log2db(logdb, "ping-servers", std::nullopt, std::nullopt, std::nullopt,
         "MONITOR", "monitor:server-activity-checker",
         (isAWarningLog ? "WARNING" : "INFO"), getpid(), std::nullopt,
         std::make_optional(output));

  ++iteration;
  micro_sleep(3000000); // Potrebbe non servire
  update_time();
}

// Controlla se una stringa rappresenta un intero positivo
bool isPositiveInteger(const char *str) {
  if (str == nullptr || strlen(str) == 0) {
    return false;
  }
  for (size_t i = 0; i < strlen(str); ++i) {
    if (!isdigit(str[i])) {
      return false;
    }
  }
  return true;
}

int main(int argc, char **argv) {

  if (argc < MIN_ARG) {
    std::cerr << "main(): usage: exec_file, pong_supplier, pong_customer, "
                 "pong_carrier, ping_supplier, ping_customer, ping_carrier, "
                 "blocking_time"
              << std::endl;
    exit(EXIT_FAILURE);
  }
  // pong streams

  if (strlen(argv[1]) > STREAM_MAX_LEN) {
    std::cerr << "Error: read_pong exceeds the maximum length of "
              << STREAM_MAX_LEN << " characters." << std::endl;
    return EXIT_FAILURE;
  }
  pong[0] = argv[1];

  if (strlen(argv[2]) > STREAM_MAX_LEN) {
    std::cerr << "Error: read_pong exceeds the maximum length of "
              << STREAM_MAX_LEN << " characters." << std::endl;
    return EXIT_FAILURE;
  }
  pong[1] = argv[2];

  if (strlen(argv[3]) > STREAM_MAX_LEN) {
    std::cerr << "Error: read_pong exceeds the maximum length of "
              << STREAM_MAX_LEN << " characters." << std::endl;
    return EXIT_FAILURE;
  }
  pong[2] = argv[3];

  // ping streams

  if (strlen(argv[4]) > STREAM_MAX_LEN) {
    std::cerr << "Error: read_ping_supplier exceeds the maximum length of "
              << STREAM_MAX_LEN << " characters." << std::endl;
    return EXIT_FAILURE;
  }
  ping[0] = argv[4];

  if (strlen(argv[5]) > STREAM_MAX_LEN) {
    std::cerr << "Error: read_ping_customer exceeds the maximum length of "
              << STREAM_MAX_LEN << " characters." << std::endl;
    return EXIT_FAILURE;
  }
  ping[1] = argv[5];

  if (strlen(argv[6]) > STREAM_MAX_LEN) {
    std::cerr << "Error: read_ping_carrier exceeds the maximum length of "
              << STREAM_MAX_LEN << " characters." << std::endl;
    return EXIT_FAILURE;
  }
  ping[2] = argv[6];

  block = atoi(argv[7]);
  if (block > MAX_BLOCKING) {
    std::cerr << "Error: block exceeds " << MAX_BLOCKING << std::endl;
    return EXIT_FAILURE;
  }

  pid = getpid();
  std::cout << "main(): pid " << pid << ": connecting to redis..." << std::endl;

  // Inizializzazione connessione a Redis
  init_stream_redis();

  std::cout << "main(): pid " << pid << ": Monitor is ready." << std::endl;

  printf("%-10s %-15s %-15s %-20s %-25s %-10s %-15s\n", "time step",
         "global_time [s]", "elapsed [s]", "present_time [ns]", "timestamp",
         "nanos", "activity");

  init_time();
  nanos = get_nanos();

  /*La mappa server_status traccia lo stato di ciascun server, utilizzando
   * valori come: 1 per attivo, -1 per ritardo, (di una sola iterazione) 0 per
   * non risponde. Inizializza i valori della mappa a 0 con
   * initializeServerStatus(). Man mano che i server inviano i loro PONG, questi
   * valori possono essere aggiornati con 1 per attivo, -1 per in ritardo oppure
   * rimanere 0
   */
  server_status = initializeServerStatus();

  while (1) {

    nanos_day = get_day_nanos(buf);

    printf("%-10ld %-15.5lf %-15.5lf %-20ld %-25s %-10ld ", iteration,
           global_time_sec, timeadvance, nanos, buf, nanos_day);

    bool ping_sent_successfully =
        true; // Variabile di controllo per il successo dell'invio dei PING

    /* Ogni server riceve il proprio messaggio PING attraverso lo stream
     * dedicato in ping[i], Viene utilizzato un ciclo for per inviare un PING a
     * ciascun server, quindi ogni server riceverà un PING indipendente che può
     * leggere e gestire senza conflitti.
     */
    for (int i = 0; i < NUM_SERVERS; i++) {
      reply = (redisReply *)redisCommand(
          c2r, "XADD %s NOMKSTREAM * monitor %ld", ping[i], iteration);

      // Verifica che il PING sia stato inviato con successo
      if (reply == nullptr) {
        std::cerr << "Error: Unable to send PING to server " << i << "."
                  << std::endl;
        freeReplyObject(reply);
        ping_sent_successfully = false;
        break;
      }
    }
    // Se l'invio di almeno un PING non ha avuto successo, salta alla prossima
    // iterazione del ciclo while
    if (!ping_sent_successfully) {
      ++iteration;
      micro_sleep(3000000);
      update_time();
      continue;
    }

    // Lettura dei PONG inviati dai server
    for (int t = 0; t < NUM_SERVERS; t++) {

      reply = (redisReply *)redisCommand(
          c2r,
          "XREADGROUP GROUP %s monitor BLOCK %d COUNT 10 NOACK STREAMS %s >",
          CONSUMER_GROUP, block, pong[t]);

      // Se non ci sono messaggi sullo stream lancia un alert e salta alla
      // prossima iterazione
      if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
        if (reply != nullptr) {
          freeReplyObject(reply);
        }
        // alertMissingServers(server_status);
        // Non serve aggiornare gli stati dei server perché la unorderd_map
        // server_status non ha subito modifiche dall'ultima inizializzazione
        continue;
      }

      // dumpReply(reply, 0);

      if (reply->type == REDIS_REPLY_ARRAY && reply->elements > 0) {

        num_msg = ReadStreamNumMsg(reply, STREAM_IND);

        for (size_t i = 0; i < num_msg; i++) { // Itera sul vettore dei messaggi

          num_val_msg = ReadStreamMsgNumVal(
              reply, STREAM_IND,
              i); // numero elementi che compongono il messaggio i-esimo

          if (num_val_msg == MAX_NUM_VAL_MSG) {

            // Estrazione dell chiave e valore del messaggio, (server_id e
            // seq_num)
            unsigned int seq_num = 0;

            ReadStreamMsgVal(
                reply, STREAM_IND, i, 0,
                server_id); // Indice 0 :  chiave contenente il server_id
            ReadStreamMsgVal(reply, STREAM_IND, i, 1,
                             value); // Indice 1 : valore di seq_num

            if ((std::string(server_id) == SERVER_SUPPLIER ||
                 std::string(server_id) == SERVER_CUSTOMER ||
                 std::string(server_id) == SERVER_CARRIER) &&
                isPositiveInteger(value)) {

              seq_num = std::stoul(value);

              if (seq_num == iteration) {

                server_status[server_id] =
                    1; // Il server_id è attivo, ha risposto entra la finestra
                       // temporale corretta
              } else if (seq_num == iteration - 1) {

                server_status[server_id] =
                    -1; // Il server_id è attivo, ma ha risposto in ritardo
              }
            } else {
              std::cerr << "Error: the message " << i
                        << " does not contain a valid key-value pair"
                        << std::endl;
            }

          } else {
            std::cerr << "Error: The message " << i
                      << " was formatted invalidly" << std::endl;
          }

        } //(size_t i = 0; i < num_msg; i++){ //Itera sul vettore dei messaggi

      } // if
      else {
        std::cerr << "Error analysing messages from servers, attempt to "
                     "reformulate in next iteration"
                  << std::endl;
        /*
        ++iteration;
        micro_sleep(3000000);
        update_time();
        */
      }
      freeReplyObject(reply);
    } // for(int i = 0; i < NUM_SERVERS; i++) //Lettura dei PONG inviati dai
      // server

    // Genera messaggi di alert per ciascun server in base allo stato (attivo,
    // in ritardo, non risponde).
    alertMissingServers(server_status);

    /* Reimposta gli stati a 0 così il monitor è pronto pronto per monitorare lo
     * stato dei serve all’interno della nuova finestra temporale.
     */
    server_status = initializeServerStatus();

  } // while(1)
  redisFree(c2r);
  return 0;
}
