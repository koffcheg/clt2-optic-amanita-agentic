Якщо залишити поточну canonical/code модель як є і не привести її до ТЗ 10.3, головний ризик не в тому, що “FrameContext інший”. Головний ризик у тому, що база знань і код будуть описувати іншу архітектуру, ніж вихідне ТЗ, а далі будь-які generated/agent-driven зміни будуть будуватися на розбіжному контракті.

**Основні ризики**

1. **Втрата трасованості даних по кадру**

   У ТЗ `FrameContext` має бути єдиною точкою, де видно весь стан кадру: raw, residual, masks, components, objects, measurements, timings. Зараз дані йдуть через окремі explicit outputs.

   Ризик: неможливо швидко відповісти “які дані були отримані на кожному етапі для цього кадру і з чого вони отримані”. Для відлагодження detection pipeline це критично.

2. **Приховане розходження між stages**

   Canonical забороняє зберігати primary outputs у context, але при цьому поточний runtime ще не має повного explicit chain. Radiometric output уже створюється, але далі не використовується як частина загального pipeline state.

   Ризик: stages можуть бути формально “правильними” окремо, але pipeline загалом не матиме authoritative frame state.

3. **LLM/code generation буде закріплювати неправильну модель**

   В AGENTS і canonical route сказано `Code = f(Cards, Stage_Spec, C)`. Якщо cards зараз говорять “FrameContext не output container”, усі майбутні AI-generated stages будуть писати код під цю модель.

   Ризик: що далі, то дорожчим стане повернення до ТЗ. Доведеться змінювати stage contracts, headers, pipeline orchestration, tests/validation, profiling і docs.

4. **Профілювання буде неповним**

   ТЗ вимагає `StageTiming` у `FrameContext`, включно з часом stages, conversions, copies. У коді зараз немає `FrameProfiling`/`StageTiming` у `FrameContext`. (ПРИМІТКА: профілювання описане, але поки не додане, це окрема задача, тобто в поточний FrameContext це додасться трохи пізніше картка dp1.domain.profiling)

   Ризик: неможливо довести відповідність RT-5/RT-20, знайти приховані `convertTo`, копіювання, дорогі allocations. Тобто performance acceptance з ТЗ буде невиконуваним.

5. **Складніше відтворювати результат**

   ТЗ вимагає reproducibility: config + context + stage data/profiling. Якщо outputs живуть поза context і не реєструються в ньому, відтворюваність залежить від зовнішнього pipeline wiring.

   Ризик: при багу measurement неможливо відновити повний шлях: який residual, яка mask, які objects дали цей measurement.

6. **Імовірність silent data loss**

   Зараз radiometric stage може повернути `ProcessingFrame`, але pipeline його не зберігає у frame-level state і не передає далі. Це вже приклад: результат є, але системно не закріплений.

   Ризик: stage output може бути обчислений і втрачений без видимого порушення контракту.

7. **Несумісність з acceptance criteria ТЗ**

   Розділ 13 вимагає стандартизовані `FrameContext`, `MeasurementRecord`, `StageProfile`, `StageTiming`; поетапне profiling; контроль переходів форматів; узгоджені структури.

   Ризик: реалізація може відповідати canonical cards, але не пройти приймання за вихідним ТЗ.

8. **Слабший контроль side effects**

   ТЗ забороняє модифікацію даних без відображення в context. Canonical explicit-output модель може бути коректною, але тільки якщо є строга ownership/provenance/reporting модель.

   Зараз такого registry/provenance шару немає.

   Ризик: з’являться локальні buffers, caches, temporary mats, які впливають на результат, але не відображаються у frame-level audit trail.

9. **Інтеграція DP1 -> DP2 стане крихкою**

   Measurement stage має розуміти photometry domain, координати, source/raw/proc references. Якщо context не зв’язує всі проміжні дані, measurement і DP2 handoff будуть залежати від набору вручну переданих посилань.

   Ризик: легко отримати measurement без зрозумілого photometry source або coordinate provenance.

10. **Документація стане внутрішньо суперечливою**

   ТЗ говорить одне, canonical cards інше, код третє: explicit-output skeleton без повного chain.

   Ризик: кожен наступний агент/розробник буде вибирати джерело істини по-своєму. Це особливо небезпечно в цьому проєкті, де canonical knowledge має бути основою генерації.

**Коли можна залишити як є**

Залишити поточну модель можна тільки якщо явно прийняти, що ТЗ 10.3 переінтерпретовано. Але тоді це потрібно оформити:

- `FrameContext` не володіє heavy payloads;
- `FrameContext` містить registry/references/provenance для всіх stage products;
- explicit outputs залишаються основним транспортом даних між stages;
- context є audit/index layer, а не storage layer;
- `StageTiming` і format conversion timing все одно мають бути в context.

Без такого рішення поточна модель залишається не “альтернативною реалізацією ТЗ”, а неузгодженим відхиленням від нього.
