import 'package:flutter/material.dart';

void main() => runApp(MaterialApp(
  home: UserPanel(),
));

class UserPanel extends StatefulWidget {
  const UserPanel({super.key});

  @override
  State<UserPanel> createState() => _UserPanelState();
}

class _UserPanelState extends State<UserPanel> {

  int _count = 0;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text ('esscale Rating System',
          style: TextStyle(
            fontSize: 40,
            fontWeight: FontWeight.bold,
            color: Colors.black,
            fontFamily: 'PoiretOne',
          )
        ),
        centerTitle: true,
        backgroundColor: Colors.green.shade400,
      ),

      body: SafeArea(
        child: Row(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Column(
              children: [
                SizedBox(
                  height: 20,
                ),
                Padding(padding: EdgeInsets.only(top: 100),),
                Text('Zayac Zaycev',
                  style: TextStyle(
                    fontSize: 30,
                    fontWeight: FontWeight.w700,
                  ),
                ),
                Padding(padding: EdgeInsets.only(top: 10),),
                CircleAvatar(
                  backgroundImage: AssetImage('assets/zayac.png'),
                  radius: 100,
                ),
                Padding(padding: EdgeInsets.only(top: 10),),
                Row(
                  children: [
                    Icon(Icons.mail_outlined, size: 20,),
                    Padding(padding: EdgeInsets.only(left: 10),),
                    Text('adminzay@mail.ru', style: TextStyle(
                      fontSize: 20,
                      fontWeight: FontWeight.w300,
                    ),),
                  ],
                ),
                Padding(padding: EdgeInsets.only(top: 10),),
                Text('Count: $_count'),
              ],
            ),
          ],
        ),
      ),

      floatingActionButton: FloatingActionButton(
        onPressed: () {
          setState(() {
            _count++;
          });
          },
        backgroundColor: Colors.amber,
        child: Icon(Icons.close_rounded),
      ),
    );
  }
}



