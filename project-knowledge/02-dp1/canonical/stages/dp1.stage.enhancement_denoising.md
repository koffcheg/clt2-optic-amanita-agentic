---
id: dp1.stage.enhancement_denoising
title:
  uk: "Етап підсилення та приглушення шуму DP1"
  en: "DP1 Enhancement and denoising stage"
tags: [dp1, canonical, stage]
kind: stage-interface-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/stages/dp1.stage.enhancement_denoising.md"
  lines: "1-130"
status: "draft"
---

## Definition

Підвищує SNR, згладжує шум, підсилює локальні структури і стабілізує сигнал перед виявленням.

## Interface

`IEnhancementStage`.

## Contract

`process(input, context, config) -> output`.

## Algorithmic idea

Покращити співвідношення сигнал/шум перед детекцією без прийняття рішення про
об’єкт і без змішування з matched filtering.

## Inputs

Домен обробки.

## Internal computation domain

Домен обробки. Рекомендований внутрішній формат: `CV_32FC1`.

## Outputs

Підсилений домен обробки: `CV_32FC1` або явно задекларований `CV_8UC1`.

## Complexity variants

- `L0`: Gaussian 3x3.
- `L1`: Gaussian 5x5 або DoG.
- `L2`: bilateral, guided або multi-scale.

## OpenCV mapping

- `GaussianBlur`: `native`.
- DoG: `composed`.
- `bilateralFilter`: `native`.
- guided або multi-scale: `wrapped` або `custom`, залежно від майбутньої
  специфікації.

## Config fragment

Ключ DSL: `enhancement`.

Обов’язкові поля: `enabled`, `variant`, `level`, `parameters`.

Допустимі `variant`: `gaussian`, `dog`, `bilateral`, `guided`, `multi_scale`.

## Timing / profiling

Профілювати час фільтрації, розмір ядра, кількість проходів, витрати на
перетворення форматів і, якщо повертаються статистики, вартість їх обчислення.

## Must not do

Фінальне рішення про виявлення або вихід вимірювань.

## Constraints

Має зберігати інформацію, потрібну наступним етапам детектора і вимірювання.

Критичні інваріанти:
- не змішувати з matched filtering;
- не створювати binary mask;
- зберігати сигнал, потрібний для detector response і photometry.

## Failure cases

Згладжування сигналу, створення штучних структур, нестабільний відгук між кадрами.

## Typical misuse

Трактувати результат denoising як binary mask.

## Open questions

Метрики якості для покращення SNR.

## Connections

- feeds: dp1.stage.matched_filtering
- uses: dp1.domain.processing
