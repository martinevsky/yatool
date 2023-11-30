## Архитектура работы yexport

 * В главном конфиге системы сборки Кроме `.CMD` некоторые модули и макросы будут через свойство `.SEM` оределять семантику.
 * Отдельный способ запуска системы сборки запускает конфигурацию с использованием `.SEM` вместо `.CMD` и выполняе дамп графа зависимостей в машиночитаемом формате (JSON).
 * В этом режиме использование в сборке макроса или модуля определяющего `.CMD` без `.SEM` рапортуется как ошибка конфигурации. При этом макросы определяющие только семантику но не команду допустимы (в нашей системе сборки мы просто выствили переменную которая проросла в какие-то комнадны, во внешней системе сборки нам может требоваться использовать какую-то эквивалентную высокоуровневую конструкцию).
 * После генерации нескольких конфигураций в JSON'ы семантик, выполняется мёрж (поиск того, что есть во всех конфигурациях, а что только в конкретных).
 * По смёрженой семантике генерируются скрипты внешних систем сборки.

### Генераторы

Для каждой целевой системы сборки делается отдельный генератор. Для cmake по историческим причинам используется отдельный генератор. Для python используется слишком ортогональный подход в экспорте и для него тоже кастомный генератор написан подом на C++. Для всех прочих случаев используется расширяемые описания генераторов на базе JINJA шаблонов.

JINJA генератор это основная часть архитектуры yexport и мы всячески стараемся его развивать и делать возможным описывать другие генераторы через этот подход. По умолчанию, если иное не оговорено, то всё написанное ниже в документе отностися к нему.

### Терминология

При описании правил сбори в yamke.core.conf к макросам добавляющим сборочные шаги мы прописывем **семантику** сборочного шага. Как правило для переменных которые хранят прописанное значение используется имена `sem`/`semantics` или производные от них.

Семантики сборочных шагов сохраняются в семантического графа и соотвествующие поля в его json представлении так же используют в своих именах `sem`/`semantics`.

В процессе анализа семантического графа собираемая из семантик информация трансформируется в иерархию **атрибутов**:
 * атрибуты всего проекта (`attrs.root`),
 * атрибуты директории в которой будет порождаться файл сборки библиотеки или приложения (`attrs.dir`),
 * атрибуты **сборочных целей** (**target**) (`attrs.target`),
 * атрибуты которые сборочные цели индуцируют на свои зивисимости (`attrs.induced`)

Таким образом от момента описания правил экспорта в конфигурационных файлах системы сборки до, составления по семантическому графу некоторого объектного представления дерева генерируемого целевого проекта мы оперируем **семантикой** сборочных правил, а в дереве проекта (в том числе в правилах по которым оно строится, описываемым в `generator.toml`) мы оперируем **атрибутами**. При выборе имён переменных в коде стоит придерживаться этой терминологии.

Формат TOML можно подробно посмотреть по ссылке https://toml.io/en/

### Особенности генераторов

Из-за особенностей взаимодействия ymake и yexport обработчик generator.toml в yexport неявно исходит из того, что все цели (targets) и все аттрибуты всех элементов (attrs) имеют уникальные строковые имена. То есть невозможно использовать цель сборки some и аттрибут some, имена должны быть уникальны на всем пространстве