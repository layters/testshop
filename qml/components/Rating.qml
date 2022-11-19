import QtQuick 2.12
import QtQuick.Controls 2.12

Row {
    property int value

    onValueChanged:{
        var stars = value
        var emptyStars = 5 - stars
        var count = 0

        while (stars > 0)
        {

            if (stars < 1)
            {
                allStars[count].value = 1
            }
            else
            {
                allStars[count].value = 2
            }
        
            count++;
            stars--;
        }

        while (emptyStars >= 1)
        {
            allStars[count].value = 0
            count++;
            emptyStars--;
        }
    }

    spacing: 5

    RatingStar {
        id: star1
    }

    RatingStar {
        id: star2
    }

    RatingStar {
        id: star3
    }

    RatingStar {
        id: star4
    }

    RatingStar {
        id: star5
    }

    property var allStars: [star1, star2, star3, star4, star5]
}