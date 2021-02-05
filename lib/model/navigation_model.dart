import 'package:flutter/material.dart';

class NavigationModel {
  String title;
  IconData icon;

  NavigationModel({this.title, this.icon});
}

List<NavigationModel> navItems = [
  NavigationModel(title: "Home", icon: Icons.home),
  NavigationModel(title: "User", icon: Icons.person),
  NavigationModel(title: "Plants", icon: Icons.grass),
  NavigationModel(title: "Analysis", icon: Icons.insert_chart_outlined),
  NavigationModel(title: "Tips", icon: Icons.book),
  NavigationModel(title: "Settings", icon: Icons.settings),
];