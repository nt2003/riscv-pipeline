.section .text
.globl _start

_start:
    addi x1, x0, 7        # x1 = 7, sanity check unrelated instruction executes normally
    jal  x5, target        # jump to target, link return addr into x5
    addi x2, x0, 99        # SHOULD be squashed — sits right behind JAL in the pipeline
    addi x2, x0, 88        # second one, in case your pipeline fetches 2 deep before squash lands
    addi x2, x0, 77        # third, extra margin — delete if your pipeline is shallower
target:
    addi x3, x0, 42        # confirms PC actually landed here
    ebreak

