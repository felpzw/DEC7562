# DEC7562 - Sistemas Operacionais Embarcados

Material da disciplina de Engenharia de Computação da UFSC/Campus Araranguá.

A resolução do primeiro trabalho prático está em `AdidOS.X/`, com mutex e herança de prioridade, escalonamento por prioridade com Round-Robin, heap com junção de blocos livres, fila alocada dinamicamente e APIs de ADC/PWM.

O [guia do trabalho](docs/primeiro-trabalho.md) reúne a correspondência com o enunciado, montagem, compilação, APIs e roteiro de demonstração com quatro tarefas.

O [manual de operação e arquitetura](docs/operacao-manual-e-arquitetura.md) explica atalhos do VS Code, PlatformIO, Proteus em VM, make/build e a organização do RTOS.

Para executar os testes locais com sanitizadores:

```sh
make -C tests test syntax
```

Para compilar o firmware com o XC16 instalado:

```sh
make -C AdidOS.X -f Makefile.pic24
```

Os testes locais validam a lógica com registradores simulados. A compilação XC16, a troca de contexto em assembly e as medições elétricas precisam ser verificadas no ambiente PIC24/Proteus.
