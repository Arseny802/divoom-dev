#pragma once

namespace divoomdev::common {

enum class display_type {
  /// up to six custom text display elements
  Text,
  /// Up to ten image display elements
  Image,
  /// Up to six network requests; You can refer to the document “https://docin.divoom-gz.com/web/#/5/145”
  NetData,
  /// Time display ()
  Time,
  /// Date display()
  Date,
  /// Weather display(Weather Must include a webp animation with 10 images, each representing a different type of
  /// weather, in the following order: sunny day，Cloudy day，Rainy day ,snow day,Frog day,sunny night ,Cloudy night
  /// ,Rainy night ,snow night ,Frog night );
  Weather,
  /// Temperature display(Temperature)
  Temperature,
  /// MonYear(2025-08)
  MonYear,
  /// Mday(the month day)
  Mday,
  Year,
  Month,
  Week,
};

}  // namespace divoomdev::common
