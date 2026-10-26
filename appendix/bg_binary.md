# bg構造のバイナリファイル構成
`bg`は非圧縮のバイナリファイルとして保存され、グラフはCSR(Compressed Sparse Row)形式のような構造を持ちます。
構造を以下に示します。

<ul>
  <li>Header
    <ul>
      <li>NodeContext position</li>
      <li>NodeContext dimension (= k)</li>
      <li>NodeIndicatorNameArray total byte count</li>
      <li>NodeIndicatorDescriptionArray total byte count</li>
      <li>WayContext position</li>
      <li>WayContext dimension (= l)</li>
      <li>WayIndicatorNameArray total byte count</li>
      <li>WayIndicatorDescriptionArray total byte count</li>
      <li>NodeArray position</li>
      <li>Node count (= m)</li>
      <li>Node byte count</li>
      <li>WayArray position</li>
      <li>Way count (= n)</li>
      <li>Way byte count</li>
    </ul>
  </li>

  <li>NodeContext
    <ul>
      <li>NodeIndicatorNameArray
        <ul>
          <li>NodeIndicatorName 1</li>
          <li>NodeIndicatorName 2</li>
          <li>...</li>
          <li>NodeIndicatorName k
            <ul>
              <li>String</li>
              <li>Null</li>
            </ul>
          </li>
        </ul>
      </li>
      <li>NodeIndicatorNameOffsetArray
        <ul>
          <li>NodeIndicatorNameOffset 1</li>
          <li>NodeIndicatorNameOffset 2</li>
          <li>...</li>
          <li>NodeIndicatorNameOffset k</li>
        </ul>
      </li>
      <li>NodeIndicatorDescriptionArray
        <ul>
          <li>NodeIndicatorDescription 1</li>
          <li>NodeIndicatorDescription 2</li>
          <li>...</li>
          <li>NodeIndicatorDescription k
            <ul>
              <li>String</li>
              <li>Null</li>
            </ul>
          </li>
        </ul>
      </li>
      <li>NodeIndicatorDescriptionOffsetArray
        <ul>
          <li>NodeIndicatorDescriptionOffset 1</li>
          <li>NodeIndicatorDescriptionOffset 2</li>
          <li>...</li>
          <li>NodeIndicatorDescriptionOffset k</li>
        </ul>
      </li>
    </ul>
  </li>

  <li>WayContext
    <ul>
      <li>WayIndicatorNameArray
        <ul>
          <li>WayIndicatorName 1</li>
          <li>WayIndicatorName 2</li>
          <li>...</li>
          <li>WayIndicatorName l
            <ul>
              <li>String</li>
              <li>Null</li>
            </ul>
          </li>
        </ul>
      </li>
      <li>WayIndicatorNameOffsetArray
        <ul>
          <li>WayIndicatorNameOffset 1</li>
          <li>WayIndicatorNameOffset 2</li>
          <li>...</li>
          <li>WayIndicatorNameOffset l</li>
        </ul>
      </li>
      <li>WayIndicatorDescriptionArray
        <ul>
          <li>WayIndicatorDescription 1</li>
          <li>WayIndicatorDescription 2</li>
          <li>...</li>
          <li>WayIndicatorDescription l
            <ul>
              <li>String</li>
              <li>Null</li>
            </ul>
          </li>
        </ul>
      </li>
      <li>WayIndicatorDescriptionOffsetArray
        <ul>
          <li>WayIndicatorDescriptionOffset 1</li>
          <li>WayIndicatorDescriptionOffset 2</li>
          <li>...</li>
          <li>WayIndicatorDescriptionOffset l</li>
        </ul>
      </li>
    </ul>
  </li>

  <li>NodeArray
    <ul>
      <li>Node 1</li>
      <li>Node 2</li>
      <li>...</li>
      <li>Node m
        <ul>
          <li>Way start idx</li>
          <li>Way end idx</li>
          <li>Longitude</li>
          <li>Latitude</li>
          <li>NodeIndicatorArray
            <ul>
              <li>NodeIndicator 1</li>
              <li>NodeIndicator 2</li>
              <li>NodeIndicator ...</li>
              <li>NodeIndicator k</li>
            </ul>
          </li>
        </ul>
      </li>
    </ul>
  </li>

  <li>WayArray
    <ul>
      <li>Way 1</li>
      <li>Way 2</li>
      <li>...</li>
      <li>Way n
        <ul>
          <li>Reverse Way idx</li>
          <li>Node connect idx</li>
          <li>Length</li>
          <li>WayIndicatorArray
            <ul>
              <li>WayIndicator 1</li>
              <li>WayIndicator 2</li>
              <li>WayIndicator ...</li>
              <li>WayIndicator l</li>
            </ul>
          </li>
        </ul>
      </li>
    </ul>
  </li>
</ul>
