import 'package:flutter/material.dart';

class NumberStepButton extends StatefulWidget {
  final int minValue;
  final int maxValue;
  final int steps;
  final ValueChanged<int> onChanged;

  const NumberStepButton(
      {Key? key,
      this.minValue = 0,
      this.maxValue = 10,
      this.steps = 1,
      required this.onChanged})
      : super(key: key);

  @override
  State<NumberStepButton> createState() => _NumberStepButtonState();
}

class _NumberStepButtonState extends State<NumberStepButton> {
  int counter = 0;

  @override
  void initState() {
    counter = widget.minValue;
    super.initState();
  }

  @override
  Widget build(BuildContext context) {
    return Row(
      mainAxisAlignment: MainAxisAlignment.spaceAround,
      children: [
        IconButton(
          icon: Icon(
            Icons.remove,
            color: Theme.of(context).colorScheme.secondary,
          ),
          padding: EdgeInsets.symmetric(vertical: 4.0, horizontal: 18.0),
          iconSize: 32.0,
          color: Theme.of(context).primaryColor,
          onPressed: () {
            setState(() {
              if (counter > widget.minValue) {
                counter -= widget.steps;
              }
              widget.onChanged(counter);
            });
          },
        ),
        Text(
          '$counter',
          textAlign: TextAlign.center,
          style: TextStyle(
            color: Colors.black87,
            fontSize: 18.0,
            fontWeight: FontWeight.w500,
          ),
        ),
        IconButton(
          icon: Icon(
            Icons.add,
            color: Theme.of(context).colorScheme.secondary,
          ),
          padding: EdgeInsets.symmetric(vertical: 4.0, horizontal: 18.0),
          iconSize: 32.0,
          color: Theme.of(context).primaryColor,
          onPressed: () {
            setState(() {
              if (counter < widget.maxValue) {
                counter += widget.steps;
              }
              widget.onChanged(counter);
            });
          },
        ),
      ],
    );
  }
}
