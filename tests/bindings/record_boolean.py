import anlnext
from submodule_python import MapExample

chain = anlnext.AnalysisChain()
chain.console = False
chain.num_parallels = 2
chain.chain(MapExample, 'records')
for key, count, enabled in [('yes', 7, True), ('no', 13, False)]:
    values = {'count': count, 'enabled': enabled}
    chain.insert_to_map('flag_table', key, values)
    chain.push_to_vector('flags', values)


def verify(c):
    expected_table = {'yes': {'count': 7, 'enabled': True}, 'no': {'count': 13, 'enabled': False}}
    expected_rows = [expected_table['yes'], expected_table['no']]
    original = c.get_parallel_module(0, 'records')
    copied = c.get_parallel_module(1, 'records')
    for module in [original, copied]:
        table = anlnext.get_value(module.get_parameter('flag_table'))
        rows = anlnext.get_value(module.get_parameter('flags'))
        assert table == expected_table and rows == expected_rows
        for row in rows + list(table.values()):
            assert type(row['enabled']) is bool and type(row['count']) is int
    extra = {'count': 23, 'enabled': True}
    anlnext.insert_to_map(copied, 'flag_table', 'extra', extra)
    anlnext.push_to_vector(copied, 'flags', extra)
    assert anlnext.get_value(copied.get_parameter('flag_table'))['extra'] == extra
    assert anlnext.get_value(copied.get_parameter('flags'))[-1] == extra
    assert anlnext.get_value(original.get_parameter('flag_table')) == expected_table
    assert anlnext.get_value(original.get_parameter('flags')) == expected_rows


chain.modify(verify)
chain.run(0)
