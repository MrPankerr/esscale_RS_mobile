import 'package:flutter/material.dart';

void main() => runApp(MyApp());

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      theme: ThemeData(primaryColor: Colors.pinkAccent),
      home:
      Scaffold(
        appBar: AppBar(
          title: Text('esscale App',
            style: TextStyle(
                fontSize: 40,
                fontWeight: FontWeight.bold,
                color: Colors.amber,
                fontFamily: 'PoiretOne'
            )
          ),
          centerTitle: true,
          backgroundColor: Colors.lightBlueAccent,
        ),

        body: Row( // или Column
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: [
            Column(
              children: [
                Text('JOPKA'),
                TextButton(onPressed: () {}, child: Text('JOPKA')),
              ],
            ),


            Column(
              children: [
                Text('JOPKA'),
                TextButton(onPressed: () {}, child: Text('JOPKA')),
              ],
            ),
          ],
        ),

        /* // Контейнеры
        Container(
          color: Colors.deepPurpleAccent,
          margin: EdgeInsets.symmetric(horizontal: 10.0, vertical: 50.0), // отступы снаружи
          padding: EdgeInsets.fromLTRB(10, 50, 10, 50), // отступы внутри
          child: Text('YaTrahal.ru'),
        ),
        */

          /* // Картинки
          child: Image(
            image: AssetImage('assets/zayac.png'),
            // image: NetworkImage('https://img-s-msn-com.akamaized.net/tenant/amp/entityid/BB1msOP8?w=0&h=0&q=60&m=6&f=jpg&u=t'),
          ),
          */

          /* // Текстовая кнопка с иконкой
          child: TextButton.icon(
              onPressed: () {},
              icon: Icon(Icons.accessible_forward),
              label: Text('Газ Газ Газ'),
          ),
          */
          
          /* // Обведенная кнопка
          child: OutlinedButton(
              onPressed: () {},
              child: Text('More, please!!!'),
          ),
          */
          
          /* // Парящая кнопка
          child: ElevatedButton(
              onPressed: () {},
              style: ButtonStyle(
                backgroundColor: WidgetStateProperty.all<Color>(Colors.black12),
              ),
              child: Text('More, please!!!'),
          ),
          */

          /* // Иконка
          child: Icon(
              Icons.settings,
              size: 45,
              color: Colors.amber,
          ),
          */


        floatingActionButton: FloatingActionButton(
          onPressed: () {},
          backgroundColor: Colors.lightBlueAccent,
          child: Text('Yamate kudosai'),
        ),

      ),
    );
  }
}