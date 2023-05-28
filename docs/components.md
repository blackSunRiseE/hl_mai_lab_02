# Компонентная архитектура

## Компонентная диаграмма
```plantuml
@startuml
!include https://raw.githubusercontent.com/plantuml-stdlib/C4-PlantUML/master/C4_Container.puml

AddElementTag("microService", $shape=EightSidedShape(), $bgColor="CornflowerBlue", $fontColor="white", $legendText="microservice")
AddElementTag("storage", $shape=RoundedBoxShape(), $bgColor="lightSkyBlue", $fontColor="white")

Person(admin, "Администратор")
Person(moderator, "Модератор")
Person(user, "Пользователь")

System_Ext(web_site, "Веб интерфейс", "HTML, CSS, JavaScript, React")

System_Boundary(conference_site, "Сайт заказа услуг") {
   Container(auth_service, "Сервис регистрации и авторизации", "C++", "Сервис для создания и авторизации пользователей", $tags = "microService")
   Container(user_service, "Сервис пользователей", "C++", "Сервис управления, редактирования и получения информации о пользователях", $tags = "microService")    
   Container(product_service, "Сервис лотов услуг", "C++", "Сервис для создание/просмотра/редактирования предоставляемых услуг", $tags = "microService")
   Container(proxysql, "proxysql", "Proxy")
   ContainerDb(db1, "db_node_01", "MySQL", "Хранение данных о пользователях и лотах услуг", $tags = "storage")
   ContainerDb(db2, "db_node_02", "MySQL", "Хранение данных о пользователях и лотах услуг", $tags = "storage")
   ContainerDb(db3, "db_node_03", "MySQL", "Хранение данных о счетчиках", $tags = "storage")
}

Rel(admin, web_site, "Просмотр, редактирование, добавление и удаление информации о пользователях и услугах.")
Rel(moderator, web_site, "Просмотр и удаление предоставляемых услуг/ блокировка пользователей")
Rel(user, web_site, "Регистрация, продажа или покупка услуг. Создание и редактирование своего лота, просмотр лотов других пользователей.")

Rel(web_site, auth_service, "Регистрация/авторизация", "localhost/auth")
Rel(auth_service, proxysql, "INSERT/SELECT/UPDATE", "SQL")

Rel(web_site, user_service, "Работа с пользователями", "localhost/user")
Rel(user_service, proxysql, "INSERT/SELECT/UPDATE", "SQL")

Rel(web_site, product_service, "Работа с услугами (создание, просмотр, редактирование)", "localhost/product")
Rel(product_service, proxysql, "INSERT/SELECT/UPDATE", "SQL")

Rel(proxysql, db1, "INSERT/SELECT/UPDATE")
Rel(proxysql, db2, "INSERT/SELECT/UPDATE")
Rel(proxysql, db3, "SELECT")

Rel(user_service, auth_service, "Валидация токена")
Rel(product_service, auth_service, "Валидация токена")

@enduml
```

# Список компонентов

Для обращения ко всем компонентам, кроме сервиса авторизации, необходим сессионный токен. Токен генерирует сервис авторизации. Токен имеет срок годности, по истечении которого необходимо генерировать новый. Тип авторизации - Bearer.
Если при обращении к сервису токен оказался просроченным, не валидным или пустым, то сервис должен вернуть код 401.

## Сервис регистрации/авторизации
**REST Api:**
- Регистрация пользователя
   - Метод - POST
   - Тело запроса - логин, пароль, фио, контактные данные
   - Тело ответа - ид созданного пользователя
   - Код успешного завершения - 201
- Авторизация пользователя
   - Метод - GET
   - Basic auth
   - Тело ответа - сессионный токен
   - Код успешного завершения - 201

## Сервис пользователей
**REST Api:**
- Редактирование пользователя
   - Метод - PUT
   - Тело запроса - ид юзера, логин, фио, контактные данные
- Поиск пользователя
   - Метод - GET
   - Параметры запроса - логин или фио или контактные данные
   - Тело ответа - ид юзера, логин, фио, контактные данные
   - Код ответа - 200
- Удаление пользователя
   - Метод - DELETE
   - Параметры запроса - id пользователя
   - Код ответа - 200

## Сервиc лотов услуг
**REST Api:**
- Создание нового лота услуги
   - Метод - Post
   - Тело запроса - название услуги, описание, информация о продавце, цена
   - Код ответа - 201
- Получение лотов услуг
   - Метод - GET
   - Параметры запроса - фильтры по названию услуги, дате создания, цене, продавцу
   - Тело ответа - ид услуги, название услуги, описание, продавец, цена, дата создания
   - Код ответа - 200
- Редактирование лота услуги
   - Метод - PUT
   - Тело запроса - ид услуги, название услуга, описание, цена
   - Код ответа - 200
- Удаление лота услуги
   - Метод - DELETE
   - Параметры запроса - id лота
   - Код ответа - 200