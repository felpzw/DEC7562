# Primeiro trabalho prático - AdidOS

Implementação para o PIC24FJ128GA010, baseada no RTOS das aulas. Números maiores representam maior prioridade. O Timer1 gera um tick de 4 ms; Timer2 e OC1 geram o PWM.

## Correspondência com o enunciado

| Requisito | Implementação | Evidência disponível |
| --- | --- | --- |
| Mutex com bloqueio | `sync.c`: inicialização, lock, unlock e transferência de posse | Testes de posse, bloqueio e desbloqueio; UART compartilhada na aplicação |
| Herança de prioridade | Prioridade base separada da efetiva; recomputação das doações após lock/unlock | Testes de cadeias, múltiplos mutexes e restauração; sequência `[PI]` na aplicação |
| Prioridade e RR no empate | `scheduler.c`: varredura circular após a tarefa atual | Testes de empates, exclusão dos bloqueados e idle; anel `scheduler_trace` |
| Junção de blocos livres | `mem.c`: união do bloco anterior e dos seguintes em `os_free` | Testes de fragmentação e integridade; autoteste de memória no boot |
| Estrutura interna dinâmica | Buffer de `m_queue_t` obtido por `os_malloc` e devolvido por `os_free` | Testes de exaustão, falha de inicialização e destruição da fila |
| API ADC | `adc.h`/`adc.c`: ADC1 de 10 bits, leitura serializada, timeout | Testes de configuração, leitura, timeout e recuperação |
| API PWM | `pwm.h`/`pwm.c`: OC1, configuração, start, stop e duty | Testes de prescaler e duty 0%, 50% e 100% |
| Aplicação com pelo menos quatro tarefas | `main.c` e `user_app.c` | Teste do boot com as quatro tarefas; roteiro experimental abaixo |
| Fila de mensagens | Sensor envia duty, controle recebe e aplica ao PWM | Testes de FIFO, retorno circular e espera por dados/espaço |
| Semáforos | Liberação do sensor e do monitor; sincronização interna da fila | Testes de espera/liberação e uso na aplicação |

## Tarefas da aplicação

| ID no trace | Tarefa | Prioridade base | Responsabilidade |
| --- | --- | --- | --- |
| 1 | `idle` | 0 | Executa quando não há tarefa de usuário pronta |
| 2 | `tarefa_adc` | 4 | Lê AN0, converte 0..1023 em 0..100%, envia à fila e registra na UART |
| 3 | `tarefa_pwm` | 2 | Recebe o duty, atualiza OC1 e libera o monitor por semáforo |
| 4 | `tarefa_monitor` | 2 | Executa junto com o controle para demonstrar RR |
| 5 | `tarefa_diagnostico` | 1 | Provoca a disputa pelo mutex da UART e registra os indicadores |

Sensor e diagnóstico compartilham `uart_mutex`. O ADC usa outro mutex interno para serializar conversões. A tarefa de controle é a única proprietária do driver PWM após o boot.

As tarefas de controle e monitor fazem trabalho ativo durante quatro ticks por amostra para permanecerem prontas juntas. O trabalho ativo no diagnóstico dura cinco ticks e permite observar a herança. As esperas normais usam fila, semáforo ou `task_delay`; esses trechos ativos são instrumentação da demonstração.

O período do sensor inclui 25 ticks de espera (100 ms), além do tempo de leitura e comunicação serial. A UART a 9600 baud limita a taxa de atualização; a aplicação não promete amostragem periódica de 100 ms exatos.

## Montagem e periféricos

Use o PIC24FJ128GA010 com alimentação de 3,3 V, AVDD/AVSS conectados e terra comum. Para protoboard, complete alimentação, desacoplamento, MCLR, regulador e programação conforme a seção 2 do [datasheet Microchip DS39747F](https://ww1.microchip.com/downloads/en/devicedoc/39747f.pdf). Confira os números físicos dos pinos para o encapsulamento utilizado.

| Sinal | Conexão para demonstração |
| --- | --- |
| AN0/RB0 | Cursor de potenciômetro de 10 kΩ; extremos em 3,3 V e GND |
| OC1/RD0 | Osciloscópio ou analisador lógico para observar PWM de 1 kHz |
| U1TX/RF3 | RX de terminal virtual ou adaptador UART com lógica de 3,3 V |
| RE0 | LED com resistor de 330 Ω: atividade do sensor |
| RE1 e RE2 | LEDs com resistores de 330 Ω: atividade das tarefas de prioridade 2 |
| INT0/RF6 | Manter em nível estável; a flag de INT0 é usada pelo RTOS para solicitar troca de contexto |

A leitura usa AVDD/AVSS como referências e AN0 como entrada. O PWM usa OC1 em RD0, sem remapeamento de pino, com Timer2. Fórmula: `fPWM = FCY / (prescaler * (PR2 + 1))`; em 1 kHz, prescaler 1 e PR2=3999. Para 100%, OC1RS recebe PR2+1; para 0%, recebe zero. Esses registradores e limites seguem as seções 14 e 21 do [datasheet](https://ww1.microchip.com/downloads/en/devicedoc/39747f.pdf).

No Proteus, adapte o circuito existente em `AdidOS.X/Proteus/Simulacao.pdsprj` com essas conexões, configure clock de 8 MHz e carregue o HEX gerado nesta branch. O projeto Proteus e os binários antigos versionados são materiais das aulas; não foram regenerados para esta solução.

## Compilação do firmware

São necessários MPLAB XC16, suporte ao PIC24FJ128GA010 e, para a interface gráfica, MPLAB X. A configuração original do repositório aponta para um compilador Windows; o makefile portátil evita esses caminhos locais:

```sh
make -C AdidOS.X -f Makefile.pic24
```

Se o compilador não estiver no PATH:

```sh
make -C AdidOS.X -f Makefile.pic24 \
  XC16_CC=/caminho/xc16/bin/xc16-gcc \
  XC16_BIN2HEX=/caminho/xc16/bin/xc16-bin2hex
```

Quando a instalação exigir um Device Family Pack externo, acrescente `DFP_DIR=/caminho/PIC24F-GA-GB_DFP/versao`; o makefile utiliza o subdiretório `xc16` desse pacote.

Os resultados são `AdidOS.X/dist/primeiro-trabalho/AdidOS.elf`, `AdidOS.hex` e `AdidOS.map`. Use o HEX novo no Proteus ou gravador. O ELF permite depuração. Não use o HEX antigo em `dist/default/production/` para avaliar esta solução.

Pela IDE, abra `AdidOS.X`, selecione o XC16 e o PIC24FJ128GA010 e regenere os makefiles da configuração antes de compilar. `adc.c`, `adc.h` e `platform.h` estão registrados em `nbproject/configurations.xml`.

`main.c` configura FRC de 8 MHz, watchdog desligado e JTAG desligado. `UART_init` define o divisor 1:1, resultando em FCY=4 MHz. O código de contexto permanece específico do PIC24, com a organização de pilha usada nas aulas. O build mantém otimização `-O0`; mudanças de compilador, otimização ou modelo de memória exigem rever esse port.

## Roteiro de demonstração

1. Compile, carregue o firmware e abra o terminal em **9600 baud, 8N1**. Reinicie o sistema para repetir a sequência de herança.
2. O sensor de prioridade 4 bloqueia no semáforo inicial. Controle e monitor também bloqueiam. O diagnóstico de prioridade 1 adquire o mutex da UART e libera o sensor.
3. O sensor tenta adquirir o mesmo mutex e bloqueia. O diagnóstico herda prioridade 4. A mensagem esperada é `[PI] prioridade herdada=4`.
4. Ao liberar o mutex, o diagnóstico transfere sua posse ao sensor e volta à prioridade 1. O sensor imprime `[PI] alta adquiriu UART apos desbloqueio`. O diagnóstico registra posteriormente `prioridade restaurada=1`.
5. Varie o potenciômetro. O terminal deve mostrar valores `ADC=... PWM=...%`. Confira no osciloscópio a frequência próxima de 1 kHz e a alteração do duty: GND produz 0%; próximo de AVDD produz 100%.
6. Observe `demo_stats.messages` e `demo_stats.pwm_duty` para conferir a comunicação pela fila. `demo_stats.adc_errors` deve permanecer zero em operação normal.
7. No depurador, observe o anel `scheduler_trace` e seu índice de próxima escrita `scheduler_trace_position`. Cada registro contém tick, ID e prioridade efetiva. Quando controle e monitor estiverem prontos juntos, os IDs **3 e 4**, ambos com prioridade **2**, devem alternar a cada tick. Desbloqueios também geram registros; compare ticks consecutivos nas janelas em que ambos estão prontos.
8. Confira `demo_stats.inherited_priority == 4`, `demo_stats.restored_priority == 1`, os dois contadores de slices crescendo e `demo_stats.heap_coalesced == true`. O autoteste mantém um bloco ocupado como barreira, libera dois blocos adjacentes e exige uma alocação maior que cada um, sem espaço suficiente na cauda.

Para examinar diretamente a herança, observe `ReadyQueue.tasks[4].base_prior` e `.task_prior`: a base permanece 1, a efetiva sobe a 4 durante a espera do sensor e retorna a 1 após a liberação. `ReadyQueue` usa índices a partir de zero; os IDs do trace começam em 1.

## Contratos das APIs

- `os_config()` informa sucesso/falha e mantém interrupções mascaradas até `os_start_scheduler()`. Crie as tarefas durante essa fase e confira os retornos de `create_task`. São permitidas `MAX_TASKS` tarefas de usuário, além da idle criada pelo kernel. Tarefas devem permanecer em loop, sem retornar.
- `task_delay(n)` recebe ticks de 4 ms. Zero equivale a yield. Prioridade fixa estrita pode deixar tarefas inferiores sem CPU se as superiores nunca bloquearem; as tarefas desta aplicação bloqueiam regularmente.
- `sem_init` e `mutex_init` são usados antes de disponibilizar o objeto. Não reinicialize objetos ativos. Semáforos atendem em FIFO e reservam fila para até `MAX_TASKS` tarefas.
- `sem_wait`, `mutex_lock`, `task_delay` e `task_yield` exigem contexto de tarefa com interrupções habilitadas. Não chame operações bloqueantes dentro de seção crítica ou ISR. As seções críticas internas restauram o estado anterior, permitindo aninhamento nas operações que não bloqueiam.
- Mutexes não são recursivos. `mutex_lock` retorna `false` para recursão, ciclo de espera ou uso inválido; quando há disputa válida, bloqueia até a posse ser transferida. `mutex_unlock` retorna `false` se a chamadora não for a proprietária. A maior prioridade efetiva é atendida primeiro; empates usam FIFO. Não há cancelamento, timeout de lock ou destruição de tarefas.
- A herança é transitiva. A prioridade efetiva é recalculada a partir das bases e de todas as tarefas esperando mutexes. Liberar um mutex preserva doações ainda necessárias por outros recursos.
- `os_heap_init` é chamado uma vez no boot. `os_malloc` retorna `NULL` para zero ou falta de memória. `os_free(NULL)` não faz nada. Libere apenas alocações válidas que ainda pertençam à chamadora; o heap tem tamanho fixo e não usa `malloc/free` da biblioteca C.
- Inicialize `m_queue_t` com `{0}`, confira `m_queue_init` e use somente filas inicializadas. Cada mensagem é um `char`; a aplicação envia um percentual de 0 a 100. Leitura e escrita bloqueiam se necessário e atualizam os índices em seção crítica, permitindo múltiplos produtores/consumidores. `m_queue_destroy` libera o buffer e exige fila vazia, sem operações de outras tarefas em andamento.
- `adc_init(canal)` configura AN0..AN15 antes do escalonador. `adc_read(&valor)` retorna `false` em timeout e libera o mutex também nesse caso. O timeout é um limite de iterações, não um prazo exato em milissegundos. Esta aplicação mantém o canal em AN0.
- Inicialize `pwm_driver_t` com `{0}`. `pwm_config` informa se a frequência é representável pelos prescalers disponíveis, podendo aproximá-la por divisão inteira. O driver representa um único OC1/Timer2. Configure antes do uso; reserve uma tarefa proprietária ou proteja o acesso com mutex. `pwm_set_duty` aceita configuração antes de start e limita valores acima de 100.

## Validação executada

```sh
make -C tests test syntax
```

O comando compila com avisos tratados como erros, AddressSanitizer e UndefinedBehaviorSanitizer e executa:

- `test_rtos`: prioridades, RR, idle, limite de tarefas, mutex e posse, herança múltipla e transitiva, restauração, prioridade/FIFO dos waiters, alinhamento/exaustão/junção do heap, integridade de dados, semáforos, seções críticas aninhadas, latência do despacho, estados de bloqueio e fila circular dinâmica;
- `test_io`: configuração e leitura ADC, timeout e recuperação, PWM e prescalers, duty de 0/50/100% e parada;
- `test_demo`: boot da aplicação real, falha de memória reportada, autoteste de junção, quatro tarefas, prioridades e configuração conjunta de ADC/PWM/UART/timer;
- `syntax`: verificação de sintaxe da aplicação, main e UART com os headers do host.

Os módulos C reais são compilados contra registradores simulados. As esperas dos testes usam pontos de suspensão controlados; eles não emulam instruções do PIC24 nem executam as quatro tarefas com preempção física.

**Resultado local:** os três executáveis e a verificação de sintaxe passaram. O makefile PIC24 também teve seus comandos inspecionados com `make -n`. **Pendente no ambiente de destino:** compilação/link com XC16, validação do port em assembly, uso real das pilhas, execução no Proteus/PIC24 e medições do ADC/PWM. XC16 e Proteus não estavam instalados no ambiente desta implementação; não há HEX novo ou captura experimental produzidos aqui.

## Organização Git

A solução usa a branch `feat/primeiro-trabalho`, com commits explicativos para núcleo, periféricos, aplicação e documentação. Stashes são locais e não são publicados com a branch.

Configurações pessoais de editor ficam separadas da solução. Para consultar e recuperar um stash nomeado sem removê-lo imediatamente:

```sh
git stash list
git stash show --stat --include-untracked 'stash@{0}'
git stash apply 'stash@{0}'
```

Confira a descrição e o conteúdo antes de aplicar: índices podem mudar quando outros stashes forem criados.
