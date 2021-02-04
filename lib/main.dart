import 'package:flutter/material.dart';
import 'package:gmd_project/model/navigation_model.dart';

void main() => runApp(GMDApp());

class GMDApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'GrowMyData App',
      home: GMDFrame(),
    );
  }
}

class GMDFrame extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        elevation: 0.0,
        backgroundColor: Color(0xFF007F0E),
        title: Center(
          child: Text("GrowMyData")
        ),
      ),
      body: Stack(
        children: <Widget> [
          Homepage(),
          NavigationDrawer(),
        ],
      )
    );
  }
}

class NavigationDrawer extends StatefulWidget {
  @override
  _NavigationDrawerState createState() => _NavigationDrawerState();
}

class _NavigationDrawerState extends State<NavigationDrawer> with SingleTickerProviderStateMixin{
  bool isCollapsed = false;
  int selectedIndex = 0;
  AnimationController _animationController;
  Animation<double> widthAnim;
  @override
  void initState() {
    super.initState();
    _animationController = AnimationController(vsync: this, duration: Duration(milliseconds: 300));
    widthAnim = Tween<double>(begin: 56, end: 200).animate(_animationController);
  }
  @override
  Widget build(BuildContext context) {
    return AnimatedBuilder(
      animation: _animationController, 
      builder: (context, widget) => getWidget(context, widget),
    );
  }
  Widget getWidget(context, widget) {
    return Container(
      width: widthAnim.value,
      decoration: BoxDecoration(
        color: Color(0xFFE5E5E5),
        border: Border(
          right: BorderSide(
            color: Color(0xFF969696),
            width: 1.0,
          )
        )
      ),
      child: Column(
        children: <Widget>[
          SizedBox(height: 20.0),
          InkWell(
            onTap: () {
              setState(() {
                isCollapsed = !isCollapsed;
                isCollapsed ? _animationController.forward() : _animationController.reverse();
              });
            },
            child: Icon(
              Icons.menu,
              color: Color(0xFF969696),
              size: 40.0,
            ),
          ),
          Expanded(
            child: ListView.builder(
              itemBuilder: (context, counter) {
                return CollapsingListTile(
                  isSelected: selectedIndex == counter,
                  onTap: () {
                    setState(() {
                      selectedIndex = counter;
                    });
                  },
                  title: navItems[counter].title,
                  icon: navItems[counter].icon,
                  animationController: _animationController
                );
              },
              itemCount: navItems.length,
            )
          ),
        ]
      )
    );
  }
}

class CollapsingListTile extends StatefulWidget {
  final String title;
  final IconData icon;
  final bool isSelected;
  final Function onTap;
  final AnimationController animationController;
  CollapsingListTile({@required this.title, @required this.icon, @required this.animationController, this.isSelected = false, this.onTap});
  @override
  _CollapsingListTileState createState() => _CollapsingListTileState();
}

class _CollapsingListTileState extends State<CollapsingListTile> {
  Animation<double> widthAnim;
  @override
  void initState() {
    super.initState();
    widthAnim = Tween<double>(begin: 56, end: 200).animate(widget.animationController);
  }
  @override
  Widget build(BuildContext context) {
    return InkWell(
      onTap: widget.onTap,
      child: Container(
        decoration: BoxDecoration(
          borderRadius: BorderRadius.all(Radius.circular(16.0)),
          color: widget.isSelected ? Colors.transparent.withOpacity(0.15) : Colors.transparent,
        ),
        width: widthAnim.value,
        margin: EdgeInsets.symmetric(horizontal: 7.0, vertical: 8.0),
        child: Row(
          children: <Widget>[
           Icon(widget.icon, color: widget.isSelected ? Color(0xFF007F0E) : Color(0xFF969696), size: 40.0,),
            SizedBox(width: (widthAnim.value >= 125) ? 10 : 0),
            (widthAnim.value >= 125) ? Text(widget.title) : Container()
          ],
        )
      )
    );
  }
}

class Homepage extends StatefulWidget {
  @override
  _HomepageState createState() => _HomepageState();
}

class _HomepageState extends State<Homepage> {
  @override
  Widget build(BuildContext context) {
    return ListView(
      padding: EdgeInsets.only(left: 56.0),
      children: <Widget> [
        Container(
          padding: EdgeInsets.all(8.0),
          height: 150,
          color: Colors.blue[500],
          child: const Center(child: Text('Test')),
        ),
        Container(
          padding: EdgeInsets.all(8.0),
          height: 150,
          color: Colors.red[500],
          child: const Center(child: Text('Test')),
        ),
      ]
    );
  }
}